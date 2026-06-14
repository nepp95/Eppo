#pragma once

#include "Asset/AssetMetadata.h"
#include "Renderer/Mesh.h"
#include "Scene/Scene.h"

namespace Eppo
{
	using importFn = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;

	class AssetImporter
	{
	public:
		static auto ImportAsset(AssetHandle handle, const AssetMetadata& metadata) -> Ref<Asset>;

		static auto ImportMesh(AssetHandle handle, const AssetMetadata& metadata) -> Ref<Mesh>;
		static auto ImportScene(AssetHandle handle, const AssetMetadata& metadata) -> Ref<Scene>;

	private:
		static std::map<AssetType, importFn> s_AssetImportFns;
	};
}