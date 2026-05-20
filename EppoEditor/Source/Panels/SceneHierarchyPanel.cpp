#include "Panels/SceneHierarchyPanel.h"

#include <imgui.h>

namespace Eppo
{
	auto SceneHierarchyPanel::RenderGui() -> void
	{
		ImGui::Begin("Scene Hierarchy");

		const auto& scene = GetSceneContext();
		for (const auto e : scene->m_Registry.view<entt::entity>())
		{
			const Entity entity(e, scene.get());
			DrawEntityNode(entity);
		}

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			SetSelectedEntity({});

		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Create new entity"))
				scene->CreateEntity("New entity");

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	auto SceneHierarchyPanel::DrawEntityNode(Entity entity) -> void
	{
		const std::string& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = (GetSelectedEntity() == entity ? ImGuiTreeNodeFlags_Selected : 0);
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	
		ImGui::PushID(entity.GetUUID());
		const bool opened = ImGui::TreeNodeEx(tag.c_str(), flags);

		if (ImGui::IsItemClicked())
			SetSelectedEntity(entity);

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Duplicate entity"))
				EP_ASSERT(false);
			if (ImGui::MenuItem("Delete entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		ImGui::PopID();

		if (opened)
			ImGui::TreePop();

		if (entityDeleted)
		{
			if (GetSelectedEntity() == entity)
				SetSelectedEntity({});

			GetSceneContext()->DestroyEntity(entity);
		}
	}
}