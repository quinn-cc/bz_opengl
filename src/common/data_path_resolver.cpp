#include "common/data_path_resolver.hpp"

#include <array>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <string>
#include <system_error>
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
            auto candidate = TryCanonical(base / relative);
            candidates.push_back(candidate);
        }
    };

    appendRelativeCandidates(exeDir);
    appendRelativeCandidates(cwd);

    std::filesystem::path fallback;

    for (const auto &candidate : candidates) {
        std::error_code ec;
        if (!std::filesystem::exists(candidate, ec) || !std::filesystem::is_directory(candidate, ec)) {
            continue;
        }

        if (fallback.empty()) {
            fallback = candidate;
        }

        const auto configPath = candidate / "config.json";
        if (std::filesystem::exists(configPath, ec)) {
            return candidate;
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
    return TryCanonical(DataRoot() / relativePath);
}

std::filesystem::path ResolveConfiguredAsset(const std::string &assetKey,
                                             const std::filesystem::path &defaultRelativePath) {
    const auto defaultPath = defaultRelativePath.empty() ? std::filesystem::path{} : Resolve(defaultRelativePath);

    static std::once_flag configLoadFlag;
    static nlohmann::json configJson;
    std::call_once(configLoadFlag, [] {
        const auto configPath = Resolve("config.json");
        std::ifstream configFile(configPath);
        if (!configFile) {
            spdlog::warn("data_path_resolver: Failed to open config file at {}.", configPath.string());
            return;
        }

        try {
            configFile >> configJson;
        } catch (const std::exception &e) {
            spdlog::error("data_path_resolver: Failed to parse config JSON: {}", e.what());
        }
    });

    if (configJson.is_object()) {
        const auto assetsIt = configJson.find("assets");
        if (assetsIt != configJson.end() && assetsIt->is_object()) {
            const auto &assets = *assetsIt;
            const auto it = assets.find(assetKey);
            if (it != assets.end() && it->is_string()) {
                return Resolve(it->get<std::string>());
            }
            spdlog::warn("data_path_resolver: Asset '{}' not found in config, using default.", assetKey);
        } else {
            spdlog::warn("data_path_resolver: Config missing 'assets' object, using default for '{}'.", assetKey);
        }
    } else if (!configJson.is_null()) {
        spdlog::warn("data_path_resolver: Config JSON is not an object, using default for '{}'.", assetKey);
    }

    return defaultPath;
}

} // namespace bz::data
