#include "Panels/PropertyPanel.h"

#include <imgui_stdlib.h>

namespace Eppo
{
	namespace Utils
	{
		template<typename T>
		static auto GetComponentString() -> std::string
		{
			const std::string fullType = typeid(T).name();
			const size_t pos = fullType.find_last_of(':');
			const size_t stringSize = fullType.size() - (pos + 1);

			return fullType.substr(pos + 1, stringSize - 9);
		}
	}

	auto PropertyPanel::RenderGui() -> void
	{
		ScopedBegin scopedBegin("Properties");

		Entity entity = GetSelectedEntity();
		if (!entity)
			return;

		DrawComponent<TagComponent>(entity, [](auto& component)
		{
			std::string& tag = component.Tag;
			std::string buffer = tag;
			if (ImGui::InputText("##Tag", &buffer))
				tag = buffer;
		});

		ImGui::SameLine();
		
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DrawAddComponentEntry<MeshComponent>("Mesh");

			ImGui::EndPopup();
		}

		DrawComponent<TransformComponent>(entity, [](auto& component)
		{
			if (ImGui::BeginTable("##", 4))
			{
				ImGui::PushID("Translation");
				ImGui::TableNextColumn();
				ImGui::Text("Translation");

				ImGui::TableNextColumn();
				if (ImGui::Button("X"))
					component.Translation.x = 0.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##TranslationX", &component.Translation.x, 0.1f);

				ImGui::TableNextColumn();
				if (ImGui::Button("Y"))
					component.Translation.y = 0.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##TranslationY", &component.Translation.y, 0.1f);

				ImGui::TableNextColumn();
				if (ImGui::Button("Z"))
					component.Translation.z = 0.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##TranslationZ", &component.Translation.z, 0.1f);

				ImGui::TableNextRow();
				ImGui::PopID();

				ImGui::PushID("Rotation");
				ImGui::TableNextColumn();
				ImGui::Text("Rotation");

				ImGui::TableNextColumn();
				if (ImGui::Button("X"))
					component.Rotation.x = 0.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##RotationX", &component.Rotation.x, 0.1f);

				ImGui::TableNextColumn();
				if (ImGui::Button("Y"))
					component.Rotation.y = 0.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##RotationY", &component.Rotation.y, 0.1f);

				ImGui::TableNextColumn();
				if (ImGui::Button("Z"))
					component.Rotation.z = 0.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##RotationZ", &component.Rotation.z, 0.1f);

				ImGui::TableNextRow();
				ImGui::PopID();

				ImGui::PushID("Scale");
				ImGui::TableNextColumn();
				ImGui::Text("Scale");

				ImGui::TableNextColumn();
				if (ImGui::Button("X"))
					component.Scale.x = 1.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##ScaleX", &component.Scale.x, 0.1f);

				ImGui::TableNextColumn();
				if (ImGui::Button("Y"))
					component.Scale.y = 1.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##ScaleY", &component.Scale.y, 0.1f);

				ImGui::TableNextColumn();
				if (ImGui::Button("Z"))
					component.Scale.z = 1.0f;
				ImGui::SameLine();
				ImGui::DragFloat("##ScaleZ", &component.Scale.z, 0.1f);

				ImGui::PopID();
				ImGui::EndTable();
			}
		});

		DrawComponent<MeshComponent>(entity, [](auto& component)
		{
			if (component.MeshHandle)
			{
				ImGui::TextDisabled(component.MeshHandle->GetName().c_str());
				ImGui::SameLine();
				if (ImGui::Button("X"))
					component.MeshHandle = nullptr;
			}
			else
			{
				ImGui::Button("Mesh Handle", ImVec2(100.0f, 0.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_ASSET"))
					{
						Ref<Mesh> handle = *reinterpret_cast<Ref<Mesh>*>(payload->Data);
						component.MeshHandle = handle;
					}
					ImGui::EndDragDropTarget();
				}
				
				if (ImGui::Button("Create mesh", ImVec2(100.0f, 0.0f)))
					ImGui::OpenPopup("CreateMesh");

				if (ImGui::BeginPopup("CreateMesh"))
				{
					if (ImGui::MenuItem("Cube"))
					{
						component.MeshHandle = Mesh::CreateCube();
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}
		});
	}

	template<typename T>
	auto PropertyPanel::DrawAddComponentEntry(const std::string& label) const -> void
	{
		if (!GetSelectedEntity().HasComponent<T>())
		{
			if (ImGui::MenuItem(label.c_str()))
			{
				GetSelectedEntity().AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	template<typename T, typename FN>
	auto PropertyPanel::DrawComponent(Entity entity, FN uiFn, const std::string& tag) -> void
	{
		if (!entity.HasComponent<T>())
			return;

		auto& c = entity.GetComponent<T>();

		std::string label = tag.empty() ? Utils::GetComponentString<T>() : tag;

		bool closedHeader = true;
		if (ImGui::CollapsingHeader(label.c_str(), &closedHeader, ImGuiTreeNodeFlags_DefaultOpen))
			uiFn(c);

		if (!closedHeader)
			entity.RemoveComponent<T>();
	}
}