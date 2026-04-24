#pragma once

#include "Core/UUID.h"

#include <entt/entt.hpp>

namespace Eppo
{
	using EntityHandle = entt::entity;
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		auto CreateEntity(const std::string& name = std::string()) -> Entity;
		auto CreateEntityWithUUID(const UUID& uuid, const std::string& name) -> Entity;
		auto DestroyEntity(Entity entity) -> void;

	private:
		entt::registry m_Registry;
		std::unordered_map<UUID, EntityHandle> m_EntityMap;

		friend class Entity;
	};
}