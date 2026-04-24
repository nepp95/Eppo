#include "pch.h"
#include "Scene/Scene.h"

#include "Scene/Components.h"
#include "Scene/Entity.h"

namespace Eppo
{
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
}