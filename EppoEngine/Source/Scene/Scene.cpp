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

	}

	auto Scene::OnRenderEditor(const std::shared_ptr<SceneRenderer>& sceneRenderer, const ScopedPtr<EditorCamera>& camera) -> void
	{
		sceneRenderer->BeginScene(camera);
		RenderScene(sceneRenderer);
		sceneRenderer->EndScene();
	}

	auto Scene::OnRenderRuntime(const std::shared_ptr<SceneRenderer>& sceneRenderer) -> void
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

	auto Scene::DestroyEntity(Entity entity) -> void
	{
		if (m_EntityMap.contains(entity.GetUUID()))
			m_EntityMap.erase(entity.GetUUID());

		m_Registry.destroy(entity);
	}

	auto Scene::RenderScene(const std::shared_ptr<SceneRenderer>& sceneRenderer) -> void
	{

	}
}