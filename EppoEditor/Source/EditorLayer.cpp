#include "EditorLayer.h"

#include "Panels/PropertyPanel.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Eppo
{
	namespace
	{
		//constexpr const char* CONTENT_BROWSER_PANEL = "Content Browser";
		constexpr const char* PROPERTY_PANEL = "Property";
		constexpr const char* SCENE_HIERARCHY_PANEL = "Scene Hierarchy";
	}

	auto EditorLayer::OnAttach() -> void
	{
		m_PanelManager = CreateRef<PanelManager>();
		m_PanelManager->AddPanel<PropertyPanel>(PROPERTY_PANEL, true);
		m_PanelManager->AddPanel<SceneHierarchyPanel>(SCENE_HIERARCHY_PANEL, true);
		//m_PanelManager->AddPanel<ContentBrowserPanel>(CONTENT_BROWSER_PANEL, true);

		m_EditorCamera = CreateScopedPtr<EditorCamera>(glm::vec3(-10.0f, 1.0f, 0.0f), 0.0f, 0.0f);

		m_ActiveScene = CreateRef<Scene>();
		auto testEntity = m_ActiveScene->CreateEntity("Test");
		auto mesh = CreateRef<Mesh>("Resources/Meshes/main_sponza/NewSponza_Main_glTF_003.gltf");
		testEntity.AddComponent<MeshComponent>(mesh);

		m_PanelManager->SetSceneContext(m_ActiveScene);
		m_SceneRenderer = CreateRef<SceneRenderer>(m_ActiveScene, m_ViewportWidth, m_ViewportHeight);
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
				if (ImGui::MenuItem("New Scene", "CTRL+N"))
					NewScene();

				if (ImGui::MenuItem("Save Scene", "CTRL+S"))
					SaveScene();

				if (ImGui::MenuItem("Open Scene", "CTRL+O"))
					OpenScene();

				if (ImGui::MenuItem("Close"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Scene render
		m_SceneRenderer->RenderGui();

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

		const auto& finalImage = m_SceneRenderer->GetFinalImage();
		ImGui::Image(ImGuiEx::CreateTextureRef(finalImage->GetTexture()), ImVec2(static_cast<float>(m_ViewportWidth), static_cast<float>(m_ViewportHeight)));

		ImGui::End(); // Viewport
		ImGui::PopStyleVar();

		ImGui::End(); // DockSpace
	}

	auto EditorLayer::OnEvent(Event& e) -> void
	{
	}

	auto EditorLayer::NewScene() -> void
	{
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_ActiveScenePath = std::filesystem::path();
		m_PanelManager->SetSceneContext(m_ActiveScene);
	}

	auto EditorLayer::OpenScene() -> bool
	{
		const auto path = FileDialog::OpenFile({
			{ "EppoEngine Scene", "epscene" }
		}, FS::GetRootDirectory());

		if (path.empty())
			return false;

		return OpenScene(path);
	}

	auto EditorLayer::OpenScene(const std::filesystem::path& path) -> bool
	{
		if (path.extension().string() != ".epscene")
		{
			Log::Error("Could not load '{}' because it is not a scene file!", path);
			return false;
		}

		const auto scene = CreateRef<Scene>();
		const SceneSerializer serializer(scene);

		if (serializer.Deserialize(path))
		{
			m_EditorScene = scene;
			m_ActiveScene = m_EditorScene;
			m_ActiveScenePath = path;
			m_PanelManager->SetSceneContext(m_ActiveScene);
		}
		else
		{
			Log::Error("Failed to deserialize scene '{}'!", path);
			return false;
		}

		return true;
	}

	auto EditorLayer::SaveScene() -> bool
	{
		if (m_ActiveScenePath.empty())
			return SaveSceneAs();

		const SceneSerializer serializer(m_ActiveScene);
		return serializer.Serialize(m_ActiveScenePath);
	}

	auto EditorLayer::SaveSceneAs() -> bool
	{
		const auto path = FileDialog::SaveFile({
			{ "EppoEngine Scene", "epscene" }
		}, FS::GetRootDirectory());

		if (path.empty())
			return false;

		m_ActiveScenePath = path;
		const SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(m_ActiveScenePath);

		return true;
	}
}