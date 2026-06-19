#include "pch.h"
#include "Scene/Scene.h"

#include "Renderer/SceneRenderer.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"

namespace Eppo
{
	auto Scene::SetViewportSize(uint32_t width, uint32_t height) -> void
	{
		// Change camera component sizzes

	}

	auto Scene::OnUpdateRuntime(float timestep) -> void
	{
		EP_PROFILE_FN("Scene::OnUpdateRuntime");
	}

	auto Scene::OnRenderEditor(const Ref<SceneRenderer>& sceneRenderer, const ScopedPtr<EditorCamera>& camera) -> void
	{
		EP_PROFILE_FN("Scene::OnRenderEditor");

		sceneRenderer->BeginScene(camera);
		RenderScene(sceneRenderer);
		sceneRenderer->EndScene();
	}

	auto Scene::OnRenderRuntime(const Ref<SceneRenderer>& sceneRenderer) -> void
	{

	}

	auto Scene::CreateEntity(const std::string& name) -> Entity
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	auto Scene::CreateEntityWithUUID(const UUID& uuid, const std::string& name) -> Entity
	{
		Entity entity(m_Registry.create(), this);

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;

		return entity;
	}

	auto Scene::DuplicateEntity(Entity entity) -> Entity
	{
		EP_PROFILE_FN("Scene::DuplicateEntity");

		const std::string& name = entity.GetName();
		const Entity newEntity = CreateEntity(name);

		TryCopyComponent<TransformComponent>(entity, newEntity);
		TryCopyComponent<MeshComponent>(entity, newEntity);

		return newEntity;
	}

	auto Scene::DestroyEntity(Entity entity) -> void
	{
		if (m_EntityMap.contains(entity.GetUUID()))
			m_EntityMap.erase(entity.GetUUID());

		m_Registry.destroy(entity);
	}

	template<typename T>
	auto Scene::TryCopyComponent(Entity srcEntity, Entity dstEntity) -> void
	{
		EP_PROFILE_FN("Scene::TryCopyComponent");

		if (srcEntity.HasComponent<T>())
			dstEntity.AddOrReplaceComponent<T>(srcEntity.GetComponent<T>());
	}

	template<typename T>
	auto Scene::CopyComponent(entt::registry& srcRegistry, entt::registry& dstRegistry, const std::unordered_map<UUID, EntityHandle>& entityMap) -> void
	{
		EP_PROFILE_FN("Scene::CopyComponent");

		auto view = srcRegistry.view<T>();
		for (auto srcEntity : view)
		{
			EntityHandle dstEntity = entityMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);
			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			dstRegistry.emplace_or_replace<T>(dstEntity, srcComponent);
		}
	}

	auto Scene::Copy(Ref<Scene> scene) -> Ref<Scene>
	{
		EP_PROFILE_FN("Scene::Copy");

		Ref<Scene> newScene = CreateRef<Scene>();

		auto& srcRegistry = scene->m_Registry;
		auto& dstRegistry = newScene->m_Registry;

		std::unordered_map<UUID, EntityHandle> entityMap;
		const auto idView = srcRegistry.view<IDComponent>();

		for (const auto entity : idView)
		{
			auto uuid = srcRegistry.get<IDComponent>(entity).ID;
			const auto& name = srcRegistry.get<TagComponent>(entity).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			entityMap[uuid] = newEntity;
		}

		CopyComponent<TransformComponent>(srcRegistry, dstRegistry, entityMap);
		CopyComponent<MeshComponent>(srcRegistry, dstRegistry, entityMap);

		return newScene;
	}

	auto Scene::RenderScene(const Ref<SceneRenderer>& sceneRenderer) -> void
	{
		EP_PROFILE_FN("Scene::RenderScene");

		const auto view = m_Registry.view<MeshComponent, TransformComponent>();
		for (const auto& entity : view)
		{
			if (auto [transformComponent, meshComponent] = view.get<TransformComponent, MeshComponent>(entity); meshComponent.MeshHandle)
			{
				sceneRenderer->SubmitMesh(meshComponent.MeshHandle, transformComponent.GetTransform());
			}
		}
	}
}