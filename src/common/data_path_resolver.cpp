#include "common/data_path_resolver.hpp"

#include <array>
#include <cstdlib>
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace {

std::filesystem::path TryCanonical(const std::filesystem::path &path) {
    std::error_code ec;
    auto result = std::filesystem::weakly_canonical(path, ec);
    if (!ec) {
        return result;
    }

    result = std::filesystem::absolute(path, ec);
    if (!ec) {
        return result;
    }

    return path;
}

std::filesystem::path ExecutableDirectory() {
#if defined(_WIN32)
    std::array<char, MAX_PATH> buffer{};
    const DWORD length = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length == buffer.size()) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(buffer.data(), buffer.data() + length).parent_path();
#elif defined(__APPLE__)
    std::array<char, PATH_MAX> buffer{};
    uint32_t size = static_cast<uint32_t>(buffer.size());
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        buffer.fill('\0');
        if (size > buffer.size()) {
            return std::filesystem::current_path();
        }
        if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
            return std::filesystem::current_path();
        }
    }
    return TryCanonical(std::filesystem::path(buffer.data())).parent_path();
#else
    std::array<char, PATH_MAX> buffer{};
    const ssize_t length = ::readlink("/proc/self/exe", buffer.data(), buffer.size());
    if (length <= 0 || static_cast<size_t>(length) >= buffer.size()) {
        return std::filesystem::current_path();
    }
    return TryCanonical(std::filesystem::path(buffer.data(), buffer.data() + length)).parent_path();
#endif
}

std::filesystem::path DetectDataRoot() {
    std::vector<std::filesystem::path> candidates;

    const char *envDataDir = std::getenv("BZ_DATA_DIR");
    if (envDataDir && *envDataDir != '\0') {
        candidates.emplace_back(TryCanonical(envDataDir));
    }

#ifdef INSTALL_DATA_DIR
    candidates.emplace_back(TryCanonical(INSTALL_DATA_DIR));
#endif

    const auto exeDir = ExecutableDirectory();
    const auto cwd = std::filesystem::current_path();

    const std::array<std::filesystem::path, 4> relativeRoots = {
        std::filesystem::path("data"),
        std::filesystem::path("../data"),
        std::filesystem::path("../../data"),
        std::filesystem::path("../../../data")
    };

    auto appendRelativeCandidates = [&](const std::filesystem::path &base) {
        for (const auto &relative : relativeRoots) {
            candidates.push_back(TryCanonical(base / relative));
        }
    };

    appendRelativeCandidates(exeDir);
    appendRelativeCandidates(cwd);

    const std::array<std::filesystem::path, 2> rootIndicators = {
        std::filesystem::path("client/config.json"),
        std::filesystem::path("server/world/Default/config.json")
    };

    std::filesystem::path fallback;

    for (const auto &candidate : candidates) {
        std::error_code ec;
        if (!std::filesystem::exists(candidate, ec) || !std::filesystem::is_directory(candidate, ec)) {
            continue;
        }

        if (fallback.empty()) {
            fallback = candidate;
        }

        for (const auto &indicator : rootIndicators) {
            const auto marker = candidate / indicator;
            if (std::filesystem::exists(marker, ec)) {
                return candidate;
            }
        }
    }

    if (!fallback.empty()) {
        return fallback;
    }

    return cwd;
}

} // namespace

namespace bz::data {

const std::filesystem::path &DataRoot() {
    static const std::filesystem::path root = DetectDataRoot();
    return root;
}

std::filesystem::path Resolve(const std::filesystem::path &relativePath) {
    if (relativePath.is_absolute()) {
        return TryCanonical(relativePath);
    }

    return TryCanonical(DataRoot() / relativePath);
}

std::filesystem::path ResolveWithBase(const std::filesystem::path &baseDir, const std::string &value) {
    std::filesystem::path candidate(value);
    if (!candidate.is_absolute()) {
        candidate = baseDir / candidate;
    }
    return TryCanonical(candidate);
}

std::filesystem::path UserConfigDirectory() {
    static const std::filesystem::path dir = [] {
        std::filesystem::path base;

#if defined(_WIN32)
        if (const char *appData = std::getenv("APPDATA"); appData && *appData) {
            base = appData;
        } else if (const char *userProfile = std::getenv("USERPROFILE"); userProfile && *userProfile) {
            base = std::filesystem::path(userProfile) / "AppData" / "Roaming";
        }
#elif defined(__APPLE__)
        if (const char *home = std::getenv("HOME"); home && *home) {
            base = std::filesystem::path(home) / "Library" / "Application Support";
        }
#else
        if (const char *xdg = std::getenv("XDG_CONFIG_HOME"); xdg && *xdg) {
            base = xdg;
        } else if (const char *home = std::getenv("HOME"); home && *home) {
            base = std::filesystem::path(home) / ".config";
        }
#endif

        if (base.empty()) {
            throw std::runtime_error("Unable to determine user configuration directory: no home path detected");
        }

        return TryCanonical(base / "bz_opengl");
    }();

    return dir;
}

std::filesystem::path EnsureUserConfigFile(const std::string &fileName) {
    const auto configDir = UserConfigDirectory();

    std::error_code ec;
    std::filesystem::create_directories(configDir, ec);
    if (ec) {
        throw std::runtime_error("Failed to create user config directory " + configDir.string() + ": " + ec.message());
    }

    const auto filePath = configDir / fileName;
    if (!std::filesystem::exists(filePath)) {
        std::ofstream stream(filePath);
        if (!stream) {
            throw std::runtime_error("Failed to create user config file " + filePath.string());
        }

        stream << "{}\n";
        if (!stream) {
            throw std::runtime_error("Failed to initialize user config file " + filePath.string());
        }
    } else if (std::filesystem::is_regular_file(filePath)) {
        std::error_code sizeEc;
        const auto fileSize = std::filesystem::file_size(filePath, sizeEc);
        if (!sizeEc && fileSize == 0) {
            std::ofstream stream(filePath, std::ios::trunc);
            if (!stream) {
                throw std::runtime_error("Failed to truncate empty user config file " + filePath.string());
            }

            stream << "{}\n";
            if (!stream) {
                throw std::runtime_error("Failed to initialize truncated user config file " + filePath.string());
            }
        }
    }

    return TryCanonical(filePath);
}

std::filesystem::path EnsureUserWorldsDirectory() {
    const auto baseDir = UserConfigDirectory();
    const auto worldsDir = baseDir / "worlds";

    std::error_code ec;
    std::filesystem::create_directories(worldsDir, ec);
    if (ec) {
        throw std::runtime_error("Failed to create user worlds directory " + worldsDir.string() + ": " + ec.message());
    }

    return TryCanonical(worldsDir);
}

std::optional<nlohmann::json> LoadJsonFile(const std::filesystem::path &path,
                                           const std::string &label,
                                           spdlog::level::level_enum missingLevel) {
    if (!std::filesystem::exists(path)) {
        spdlog::log(missingLevel, "data_path_resolver: {} not found: {}", label, path.string());
        return std::nullopt;
    }

    std::ifstream stream(path);
    if (!stream) {
        spdlog::error("data_path_resolver: Failed to open {}: {}", label, path.string());
        return std::nullopt;
    }

    try {
        nlohmann::json json;
        stream >> json;
        return json;
    } catch (const std::exception &e) {
        spdlog::error("data_path_resolver: Failed to parse {}: {}", label, e.what());
        return std::nullopt;
    }
}

std::vector<ConfigLayer> LoadConfigLayers(const std::vector<ConfigLayerSpec> &specs) {
    std::vector<ConfigLayer> layers;
    layers.reserve(specs.size());

    for (const auto &spec : specs) {
        const auto absolutePath = Resolve(spec.relativePath);
        const std::string label = spec.label.empty() ? spec.relativePath.string() : spec.label;
        auto jsonOpt = LoadJsonFile(absolutePath, label, spec.missingLevel);
        if (!jsonOpt) {
            if (spec.required) {
                spdlog::error("data_path_resolver: Required config missing: {}", absolutePath.string());
            }
            continue;
        }

        if (!jsonOpt->is_object()) {
            spdlog::warn("data_path_resolver: Config {} is not a JSON object, skipping", absolutePath.string());
            continue;
        }

        layers.push_back({std::move(*jsonOpt), absolutePath.parent_path()});
    }

    return layers;
}

void MergeJsonObjects(nlohmann::json &destination, const nlohmann::json &source) {
    if (!destination.is_object() || !source.is_object()) {
        destination = source;
        return;
    }

    for (auto it = source.begin(); it != source.end(); ++it) {
        const auto &key = it.key();
        const auto &value = it.value();

        if (value.is_object() && destination.contains(key) && destination[key].is_object()) {
            MergeJsonObjects(destination[key], value);
        } else {
            destination[key] = value;
        }
    }
}

void CollectAssetEntries(const nlohmann::json &node,
                         const std::filesystem::path &baseDir,
                         std::map<std::string, std::filesystem::path> &assetMap,
                         const std::string &prefix) {
    if (!node.is_object()) {
        return;
    }

    for (const auto &[key, value] : node.items()) {
        const std::string fullKey = prefix.empty() ? key : prefix + "." + key;
        if (value.is_string()) {
            assetMap[fullKey] = ResolveWithBase(baseDir, value.get<std::string>());
        } else if (value.is_object()) {
            CollectAssetEntries(value, baseDir, assetMap, fullKey);
        }
    }
}

std::filesystem::path ResolveConfiguredAsset(const std::string &assetKey,
                                             const std::filesystem::path &defaultRelativePath) {
    const auto defaultPath = defaultRelativePath.empty() ? std::filesystem::path{} : Resolve(defaultRelativePath);

    static std::once_flag configLoadFlag;
    static std::unordered_map<std::string, std::filesystem::path> assetLookup;
    std::call_once(configLoadFlag, [] {
        const auto userConfigPath = EnsureUserConfigFile("config.json");

        const std::vector<ConfigLayerSpec> specs = {
            {"common/config.json", "data/common/config.json", spdlog::level::err, false},
            {"client/config.json", "data/client/config.json", spdlog::level::debug, false},
            {userConfigPath, "user config", spdlog::level::debug, false}
        };

        const auto layers = LoadConfigLayers(specs);

        std::map<std::string, std::filesystem::path> flattened;
        for (const auto &layer : layers) {
            if (layer.json.is_object()) {
                const auto assetsIt = layer.json.find("assets");
                if (assetsIt != layer.json.end()) {
                    if (!assetsIt->is_object()) {
                        spdlog::warn("data_path_resolver: 'assets' in {} is not an object; skipping", layer.baseDir.string());
                    } else {
                        CollectAssetEntries(*assetsIt, layer.baseDir, flattened);
                    }
                }

                const auto fontsIt = layer.json.find("fonts");
                if (fontsIt != layer.json.end()) {
                    if (!fontsIt->is_object()) {
                        spdlog::warn("data_path_resolver: 'fonts' in {} is not an object; skipping", layer.baseDir.string());
                    } else {
                        CollectAssetEntries(*fontsIt, layer.baseDir, flattened, "fonts");
                    }
                }
            }
        }

        for (const auto &[key, resolvedPath] : flattened) {
            assetLookup[key] = resolvedPath;

            const auto separator = key.find_last_of('.');
            if (separator != std::string::npos) {
                assetLookup[key.substr(separator + 1)] = resolvedPath;
            }
        }
    });

    if (const auto it = assetLookup.find(assetKey); it != assetLookup.end()) {
        return it->second;
    }

    spdlog::warn("data_path_resolver: Asset '{}' not found in configuration layers, using default.", assetKey);
    return defaultPath;
}

} // namespace bz::data
