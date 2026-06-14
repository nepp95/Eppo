#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMetadata.h"

#include <future>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace Eppo
{
	class AssetManager
	{
	public:
		auto CreateAsset(const std::filesystem::path& path, const Ref<Asset>& existingAsset = nullptr) -> bool;
		auto GetAsset(AssetHandle handle, bool async = false) -> Ref<Asset>;

		template<typename T>
			requires(std::derived_from<T, Asset>)
		auto GetAsset(AssetHandle handle, bool async = false) -> Ref<T>
		{
			return std::static_pointer_cast<T>(GetAsset(handle, async));
		}

		auto Tick() -> void;

		[[nodiscard]] auto HasAssetMetadata(AssetHandle handle) const -> bool;
		[[nodiscard]] auto IsAssetHandleValid(AssetHandle handle) const -> bool;
		[[nodiscard]] auto IsAssetLoaded(AssetHandle handle) const -> bool;
		[[nodiscard]] auto GetMetadata(AssetHandle handle) const -> const AssetMetadata&;

		[[nodiscard]] auto GetAssetRegistry() const -> const std::map<AssetHandle, AssetMetadata>& { return m_AssetData; };
		auto SerializeAssetRegistry() const -> void;
		auto DeserializeAssetRegistry() -> bool;

	private:
		std::map<AssetHandle, AssetMetadata> m_AssetData;
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
		mutable std::shared_mutex m_Mutex;
	};
}