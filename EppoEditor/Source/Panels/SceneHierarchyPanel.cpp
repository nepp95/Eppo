#include "Panels/SceneHierarchyPanel.h"

#include <imgui.h>

namespace Eppo
{
	auto SceneHierarchyPanel::RenderGui() -> void
	{
		ImGui::Begin("Scene Hierarchy");
		ImGui::Text("Scene Hierarchy");
		ImGui::End();
	}
}