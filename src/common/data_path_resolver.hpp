#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace bz::data {

// Resolve paths located under the runtime data directory.
std::filesystem::path Resolve(const std::filesystem::path &relativePath);

std::optional<nlohmann::json> LoadJsonFile(const std::filesystem::path &path,
										   const std::string &label,
										   spdlog::level::level_enum missingLevel);

std::filesystem::path UserConfigDirectory();
std::filesystem::path EnsureUserConfigFile(const std::string &fileName);
std::filesystem::path EnsureUserWorldsDirectory();

struct ConfigLayerSpec {
	std::filesystem::path relativePath;
	std::string label;
	spdlog::level::level_enum missingLevel = spdlog::level::warn;
	bool required = false;
};

struct ConfigLayer {
	nlohmann::json json;
	std::filesystem::path baseDir;
};

std::vector<ConfigLayer> LoadConfigLayers(const std::vector<ConfigLayerSpec> &specs);

void MergeJsonObjects(nlohmann::json &destination, const nlohmann::json &source);

void CollectAssetEntries(const nlohmann::json &node,
						 const std::filesystem::path &baseDir,
						 std::map<std::string, std::filesystem::path> &assetMap,
						 const std::string &prefix = "");

// Resolve an asset path declared in client/config.json, falling back to a default relative path.
std::filesystem::path ResolveConfiguredAsset(const std::string &assetKey,
											 const std::filesystem::path &defaultRelativePath = {});

// Returns the detected runtime data directory.
const std::filesystem::path &DataRoot();

} // namespace bz::data
