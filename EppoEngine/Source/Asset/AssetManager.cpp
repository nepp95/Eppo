#include "pch.h"
#include "Asset/AssetManager.h"

#include "Asset/AssetImporter.h"
#include "Project/Project.h"
#include "Utility/Json.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Eppo
{
    namespace
    {
        const AssetMetadata s_NullMetadata;

        auto DeduceAssetTypeFromExtension(const std::filesystem::path& path) -> AssetType
        {
			const auto ext = path.extension().string();

			if (ext == ".gltf" || ext == ".glb")
				return AssetType::Mesh;
            if (ext == ".epscene")
                return AssetType::Scene;

			return AssetType::None;
        }
    }

    auto AssetManager::CreateAsset(const std::filesystem::path& path, const Ref<Asset>& existingAsset) -> bool
    {
        EP_PROFILE_FN("AssetManager::CreateAsset");

        const AssetHandle handle = existingAsset ? existingAsset->Handle : UUID();
        const AssetMetadata metadata{
			.Handle = handle,
			.Type = DeduceAssetTypeFromExtension(path),
			.Filepath = Project::GetAssetRelativeFilepath(path),
        };

        {
            std::scoped_lock lock(m_Mutex);

            if (m_AssetData.contains(handle))
            {
                Log::Error("Asset already contained in asset manager!");
                return false;
            }
            else
            {
                m_AssetData[handle] = metadata;
            }
        }

        SerializeAssetRegistry();

        return true;
    }

    auto AssetManager::GetAsset(AssetHandle handle, bool async) -> Ref<Asset>
    {
        EP_PROFILE_FN("AssetManager::LoadAsset");

        std::scoped_lock lock(m_Mutex);

        // Asset already loaded
	    if (m_LoadedAssets.contains(handle))
		    return m_LoadedAssets.at(handle);

        // Create asset instance
        if (!m_AssetData.contains(handle))
        {
            Log::Error("Failed to load asset with handle '{}'", handle);
            return nullptr;
        }

	    const auto& metadata = m_AssetData.at(handle);

	    Ref<Asset> asset = CreateRef<Asset>();

        if (async)
        {
        }
        else
        {
            // Load asset here
            asset = AssetImporter::ImportAsset(handle, metadata);
        }

        asset->Handle = handle;
        m_LoadedAssets[handle] = asset;

        return asset;
    }

    auto AssetManager::Tick() -> void
    {
    }

	auto AssetManager::HasAssetMetadata(AssetHandle handle) const -> bool
	{
        return m_AssetData.contains(handle);
	}

	auto AssetManager::IsAssetHandleValid(AssetHandle handle) const -> bool
    {
        return HasAssetMetadata(handle) && m_AssetData.at(handle).IsValid();
    }

    auto AssetManager::IsAssetLoaded(AssetHandle handle) const -> bool
    {
        return m_AssetData.contains(handle) && m_LoadedAssets.contains(handle);
    }

    auto AssetManager::GetMetadata(AssetHandle handle) const -> const AssetMetadata&
    {
        if (m_AssetData.contains(handle))
            return m_AssetData.at(handle);
        return s_NullMetadata;
    }

	auto AssetManager::SerializeAssetRegistry() const -> void
    {
        EP_PROFILE_FN("AssetManager::SerializeAssetRegistry");

        std::shared_lock lock(m_Mutex);

        json data;
        data["Assets"] = nlohmann::json::array();
        for (const auto& [handle, metadata] : m_AssetData)
        {
            json asset;
            asset["Handle"] = handle;
            asset["Type"] = Utils::AssetTypeToString(metadata.Type);
            asset["Filepath"] = metadata.Filepath.string();
            data["Assets"].emplace_back(asset);
        }

        lock.unlock();
        FS::WriteText(Project::GetAssetsDirectory() / "AssetRegistry.json", data.dump(4), true);
    }

	auto AssetManager::DeserializeAssetRegistry() -> bool
	{
        EP_PROFILE_FN("AssetManager::DeserializeAssetRegistry");

		const auto path = Project::GetAssetsDirectory() / "AssetRegistry.json";
		if (!FS::Exists(path))
		{
			Log::Error("Could not deserialize asset registry!");
			return false;
		}

		std::ifstream stream(path);
		json data;

		try
		{
			data = json::parse(stream);
		}
		catch (const json::exception& ex)
		{
			Log::Error("Failed to parse asset registry file '{}'!", path);
			Log::Error("Parse error: {}", ex.what());
			return false;
		}

		if (!data.contains("Assets"))
			return false;

		std::scoped_lock lock(m_Mutex);

		for (const auto& e : data["Assets"])
		{
			const AssetHandle handle = e["Handle"].get<UUID>();

			const AssetMetadata metadata{
				.Handle = handle,
				.Type = Utils::AssetTypeFromString(e["Type"].get<std::string>()),
				.Filepath = e["Filepath"].get<std::string>(),
			};

			m_AssetData[handle] = metadata;
		}

		return true;
	}
}