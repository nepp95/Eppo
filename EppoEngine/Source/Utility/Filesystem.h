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
	
	inline auto GetShaderCacheDirectory() -> std::filesystem::path
	{
		return GetResourcesDirectory() / "Shaders" / "Cache";
	}

	inline auto CreateDirectory(const std::filesystem::path& path) -> bool
	{
		return std::filesystem::create_directories(path);
	}

	inline auto Exists(const std::filesystem::path& path) -> bool
	{
		return std::filesystem::exists(path);
	}

	inline auto Copy(const std::filesystem::path& from, const std::filesystem::path& to) -> bool
	{
		if (!Exists(from))
		{
			Log::Error("Failed to copy from '{}' because the path does not exist!", from);
			return false;
		}

		std::filesystem::copy(from, to);
		return true;
	}

	inline auto Move(const std::filesystem::path& from, const std::filesystem::path& to) -> bool
	{
		if (!Exists(from))
		{
			Log::Error("Failed to copy from '{}' because the path does not exist!", from);
			return false;
		}

		std::filesystem::rename(from, to);
		return true;
	}

	inline auto ReadBytes(const std::filesystem::path& path) -> std::vector<char>
	{
		// Open file stream
		std::ifstream in(path, std::ios::binary);
		if (!in)
		{
			Log::Error("Failed to open file stream for path '{}'", path);
			return {};
		}

		// Get file size
		const auto fileSize = std::filesystem::file_size(path);
		if (fileSize == 0)
		{
			Log::Error("Failed to read bytes because file size was zero for path '{}'", path);
			return {};
		}

		std::vector<char> bytes(fileSize);
		in.read(bytes.data(), fileSize);

		return bytes;
	}

	inline auto ReadText(const std::filesystem::path& path) -> std::string
	{
		// Open file stream
		std::ifstream in(path, std::ios::binary);
		if (!in)
		{
			Log::Error("Failed to open file stream for path '{}'", path);
			return {};
		}

		// Get file size
		const auto fileSize = std::filesystem::file_size(path);
		if (fileSize == 0)
		{
			Log::Error("Failed to read text because file size was zero for path '{}'", path);
			return {};
		}

		std::string text;
		text.resize(fileSize);
		in.read(text.data(), text.size());

		return text;
	}
}