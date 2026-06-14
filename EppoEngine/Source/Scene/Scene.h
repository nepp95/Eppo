#pragma once

#include "Asset/Asset.h"
#include "Core/UUID.h"
#include "Renderer/Camera/EditorCamera.h"

#include <entt/entt.hpp>

namespace Eppo
{
	using EntityHandle = entt::entity;
	class Entity;
	class SceneRenderer;

	class Scene : public Asset
	{
	public:
		Scene() = default;
		~Scene() = default;

		auto SetViewportSize(uint32_t width, uint32_t height) -> void;

		auto OnUpdateRuntime(float timestep) -> void;

		auto OnRenderEditor(const Ref<SceneRenderer>& sceneRenderer, const ScopedPtr<EditorCamera>& camera) -> void;
		auto OnRenderRuntime(const Ref<SceneRenderer>& sceneRenderer) -> void;

		auto CreateEntity(const std::string& name = std::string()) -> Entity;
		auto CreateEntityWithUUID(const UUID& uuid, const std::string& name) -> Entity;
		auto DestroyEntity(Entity entity) -> void;

	private:
		auto RenderScene(const Ref<SceneRenderer>& sceneRenderer) -> void;

	private:
		entt::registry m_Registry;
		std::unordered_map<UUID, EntityHandle> m_EntityMap;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};
}