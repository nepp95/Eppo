#pragma once

namespace Eppo
{
	enum class AssetType
	{
		None = 0,
		Mesh,
		Scene,
	};

	namespace Utils
	{
		constexpr auto AssetTypeFromString(const std::string_view assetType) -> AssetType
		{
			if (assetType == "None")
				return AssetType::None;
			if (assetType == "Mesh")
				return AssetType::Mesh;
			if (assetType == "Scene")
				return AssetType::Scene;

			EP_ASSERT(false);
			return AssetType::None;
		}

		constexpr auto AssetTypeToString(const AssetType& type) -> std::string
		{
			switch (type)
			{
				case AssetType::None:
					return "None";
				case AssetType::Mesh:
					return "Mesh";
				case AssetType::Scene:
					return "Scene";
				default:
				{
					EP_ASSERT(false);
					return "None";
				}
			}
		}
	}
}