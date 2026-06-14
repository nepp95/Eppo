#include "pch.h"
#include "Asset/AssetImporter.h"

#include "Asset/AssetManager.h"
#include "Scene/SceneSerializer.h"
#include "Project/Project.h"

namespace Eppo
{
	std::map<AssetType, importFn> AssetImporter::s_AssetImportFns = {
		{ AssetType::Scene, AssetImporter::ImportScene }
	};

	auto AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata) -> Ref<Asset>
	{
		EP_PROFILE_FN("AssetImporter::ImportAsset");

		if (!s_AssetImportFns.contains(metadata.Type))
		{
			Log::Error("No importer available for asset type: {}", Utils::AssetTypeToString(metadata.Type));
			return nullptr;
		}

		return s_AssetImportFns.at(metadata.Type)(handle, metadata);
	}

	auto AssetImporter::ImportMesh(AssetHandle handle, const AssetMetadata& metadata) -> Ref<Mesh>
	{
		EP_PROFILE_FN("AssetImporter::ImportMesh");
	}

	auto AssetImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata) -> Ref<Scene>
	{
		EP_PROFILE_FN("AssetImporter::ImportScene");

		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(Project::GetAssetFilepath(metadata.Filepath));

		return scene;
	}
}