#pragma once

#include <filesystem>

namespace Eppo::FS
{
	inline auto GetRootDirectory() -> std::filesystem::path
	{
		return std::filesystem::current_path();
	}

	inline auto GetResourcesDirectory() -> std::filesystem::path
	{
		return GetRootDirectory() / "Resources";
	}

	inline auto CreateDirectory() -> bool
	{

	}

	inline auto Copy(const std::filesystem::path& from, const std::filesystem::path& to) -> bool
	{

	}

	inline auto Move(const std::filesystem::path& from, const std::filesystem::path& to) -> bool
	{

	}

	inline auto Exists(const std::filesystem::path& path) -> bool
	{

	}
}