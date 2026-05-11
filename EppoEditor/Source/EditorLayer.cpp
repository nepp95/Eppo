#include "EditorLayer.h"

#include "Panels/SceneHierarchyPanel.h"

namespace Eppo
{
	namespace
	{
		constexpr const char* SCENE_HIERARCHY_PANEL = "Scene Hierarchy";
		constexpr const char* CONTENT_BROWSER_PANEL = "Content Browser";
	}

	auto EditorLayer::OnAttach() -> void
	{
		m_PanelManager = std::make_shared<PanelManager>();
		m_PanelManager->AddPanel<SceneHierarchyPanel>(SCENE_HIERARCHY_PANEL, true);
		//m_PanelManager->AddPanel<ContentBrowserPanel>(CONTENT_BROWSER_PANEL, true);

		m_EditorCamera = CreateScopedPtr<EditorCamera>(glm::vec3(-10.0f, 1.0f, 0.0f), 0.0f, 0.0f);

		m_ActiveScene = std::make_shared<Scene>();
		m_ActiveScene->CreateEntity("Test");

		m_SceneRenderer = std::make_shared<SceneRenderer>(m_ActiveScene);
	}

	auto EditorLayer::OnDetach() -> void
	{
	}

	auto EditorLayer::OnUpdate(float timestep) -> void
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
		{
			m_EditorCamera->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			m_ActiveScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			m_SceneRenderer->Resize(m_ViewportWidth, m_ViewportHeight);
		}

		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_EditorCamera->OnUpdate(timestep);
				m_ActiveScene->OnRenderEditor(m_SceneRenderer, m_EditorCamera);
				break;
			}

			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(timestep);
				m_ActiveScene->OnRenderRuntime(m_SceneRenderer);
				break;
			}
		}
	}

	auto EditorLayer::OnUIRender() -> void
	{
		// From ImGui docking example
		bool dockspaceOpen = true;
		constexpr ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspaceOpen, windowFlags);
		ImGui::PopStyleVar(3);

		const ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		const float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 200.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}

		style.WindowMinSize.x = minWinSizeX;

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Close"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Panels
		m_PanelManager->RenderGui();

		// Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		const auto& app = Application::Get();
		app.GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

		const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportWidth = static_cast<uint32_t>(viewportSize.x);
		m_ViewportHeight = static_cast<uint32_t>(viewportSize.y);

		ImGui::End(); // Viewport
		ImGui::PopStyleVar();

		ImGui::End(); // DockSpace
	}

	auto EditorLayer::OnEvent(Event& e) -> void
	{
	}
}