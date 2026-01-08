#pragma once

#include <filesystem>
#include <string>

namespace bz::data {

// Resolve paths located under the runtime data directory.
std::filesystem::path Resolve(const std::filesystem::path &relativePath);

// Resolve an asset path declared in config.json->assets, falling back to a default relative path.
std::filesystem::path ResolveConfiguredAsset(const std::string &assetKey,
											 const std::filesystem::path &defaultRelativePath = {});

// Returns the detected runtime data directory.
const std::filesystem::path &DataRoot();

} // namespace bz::data
