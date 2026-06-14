#include "pch.h"
#include "Scene/SceneSerializer.h"

#include "Utility/Json.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Eppo
{
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_SceneContext(scene)
	{}

	auto SceneSerializer::Serialize(const std::filesystem::path& path) const -> bool
	{
		EP_PROFILE_FN("SceneSerializer::Serialize");

		std::string sceneName = path.stem().string();
		Log::Info("Serializing scene '{}'", sceneName);

		json data;
		data["Scene"]["Name"] = sceneName;
		data["Scene"]["Handle"] = m_SceneContext->Handle;
		auto entities = json::array();

		m_SceneContext->m_Registry.sort<IDComponent>(
			[](const auto& lhs, const auto& rhs) 
			{
				return lhs.ID < rhs.ID;
			}
		);
		
		const auto view = m_SceneContext->m_Registry.view<IDComponent>();
		for (const auto e : view)
		{
			const Entity entity(e, m_SceneContext.get());
			if (!entity)
				continue;

			SerializeEntity(entities, entity);
		}

		data["Scene"]["Entities"] = entities;
		FS::WriteText(path, data.dump(4), true);

		return true;
	}

	auto SceneSerializer::Deserialize(const std::filesystem::path& path) const -> bool
	{
		EP_PROFILE_FN("SceneSerializer::Deserialize");

		std::ifstream stream(path);
		json data;

		try
		{
			data = json::parse(stream);
		}
		catch (const json::exception& ex)
		{
			Log::Error("Failed to parse scene file '{}'!", path);
			Log::Error("Parse error: {}", ex.what());
			return false;
		}

		const auto sceneName = data["Scene"]["Name"].get<std::string>();
		Log::Info("Deserializing scene '{}'", sceneName);

		auto& entities = data["Scene"]["Entities"];
		if (entities.empty())
		{
			Log::Warn("Scene '{}' has no entities, are you sure this is correct?", sceneName);
			return true;
		}

		for (auto& entity : entities)
		{
			// ID
			if (!entity.contains("IDComponent"))
			{
				Log::Warn("Entity did not have a unique identifier, skipping...");
				continue;
			}

			const auto entityId = entity["IDComponent"]["ID"].get<UUID>();

			// Tag
			if (!entity.contains("TagComponent"))
			{
				Log::Warn("Entity did not have a tag, skipping...");
				continue;
			}

			const auto entityName = entity["TagComponent"]["Tag"].get<std::string>();

			// Create new entity
			Entity newEntity = m_SceneContext->CreateEntityWithUUID(entityId, entityName);
			Log::Info("Deserializing entity '{}' ({})", entityName, entityId);

			if (entity.contains("TransformComponent"))
			{
				auto& c = entity["TransformComponent"];
				auto& nc = newEntity.GetComponent<TransformComponent>();
				nc.Translation = c["Translation"].get<glm::vec3>();
				nc.Rotation = c["Rotation"].get<glm::vec3>();
				nc.Scale = c["Scale"].get<glm::vec3>();
			}

			if (entity.contains("MeshComponent"))
			{

			}
		}

		return true;
	}

	auto SceneSerializer::SerializeEntity(nlohmann::json& data, Entity entity) const -> void
	{
		EP_PROFILE_FN("SceneSerializer::SerializeEntity");
		EP_ASSERT(entity.HasComponent<IDComponent>() && entity.HasComponent<TagComponent>());

		Log::Info("Serializing entity '{}' ({})", entity.GetName(), entity.GetUUID());
		json e;

		e["IDComponent"]["ID"] = entity.GetUUID();
		e["TagComponent"]["Tag"] = entity.GetName();

		if (entity.HasComponent<TransformComponent>())
		{
			const auto& c = entity.GetComponent<TransformComponent>();
			e["TransformComponent"]["Translation"] = c.Translation;
			e["TransformComponent"]["Rotation"] = c.Rotation;
			e["TransformComponent"]["Scale"] = c.Scale;
		}

		if (entity.HasComponent<MeshComponent>())
		{
			const auto& c = entity.GetComponent<MeshComponent>();
			// TODO: Implement this
		}

		data.emplace_back(e);
	}
}