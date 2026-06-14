#include "EditorLayer.h"

#include "Panels/PropertyPanel.h"
#include "Panels/SceneHierarchyPanel.h"

#include <imgui_stdlib.h>

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

		if (!OpenProject())
		{
			m_ActiveScene = CreateRef<Scene>();
			m_PanelManager->SetSceneContext(m_ActiveScene);
		}

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
				if (ImGui::MenuItem("New Project", "CTRL+N"))
					m_NewProjectPopup = true;

				if (ImGui::MenuItem("Save Project", "CTRL+S"))
					SaveProject();

				if (ImGui::MenuItem("Open Project", "CTRL+O"))
					OpenProject();

				if (ImGui::MenuItem("Close Project"))
					CloseProject();

				if (ImGui::MenuItem("New Scene"))
					NewScene();

				if (ImGui::MenuItem("Save Scene"))
					SaveScene();

				if (ImGui::MenuItem("Open Scene"))
					OpenScene();

				if (ImGui::MenuItem("Close"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Popups
		if (m_NewProjectPopup)
		{
			constexpr ImGuiPopupFlags popupFlags = ImGuiPopupFlags_NoOpenOverExistingPopup;
			ImGui::OpenPopup("New Project", popupFlags);
			m_NewProjectPopup = false;
		}

		UI_NewProjectPopup();

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
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(std::bind_front(&EditorLayer::OnKeyPressed, this));
	}

	auto EditorLayer::OnKeyPressed(const KeyPressedEvent& e) -> bool
	{
		if (e.IsRepeat())
			return false;

		[[maybe_unused]] const bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		[[maybe_unused]] const bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		[[maybe_unused]] const bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (control)
					m_NewProjectPopup = true;
			}

			case Key::O:
			{
				if (control)
					OpenProject();
				break;
			}

			case Key::S:
			{
				if (control)
					SaveProject();
				break;
			}
		}

		return false;
	}

	auto EditorLayer::CloseProject() -> void
	{
		SaveProject();

		m_PanelManager->SetSceneContext(nullptr);
		
		if (Project::GetActive())
			Project::SetActive(nullptr);

		m_EditorScene = nullptr;
		m_ActiveScene = nullptr;
	}

	auto EditorLayer::NewProject(const std::string& name) -> void
	{
		// Create project directory
		const auto projectPath = FS::GetRootDirectory() / "Projects" / name;
		FS::CreateDirectory(projectPath);

		// Copy new project template
		FS::Copy("Resources/Templates/NewProject", projectPath);

		// Create directories
		FS::CreateDirectory(projectPath / "Assets" / "Meshes");
		FS::CreateDirectory(projectPath / "Assets" / "Scenes");

		// Replace tokens
		constexpr auto ReplaceToken = [](std::string& input, const char* token, const std::string& value) -> void
		{
			size_t pos = 0;
			while ((pos = input.find(token, pos)) != std::string::npos)
			{
				input.replace(pos, strlen(token), value);
				pos += strlen(token);
			}
		};

		{
			auto inputStr = FS::ReadText(projectPath / "project.epproj");
			ReplaceToken(inputStr, "$PROJECT_NAME$", name);
			FS::WriteText(projectPath / "project.epproj", inputStr, true);
			FS::Move(projectPath / "project.epproj", projectPath / std::filesystem::path(name + ".epproj"));
		}

		OpenProject(projectPath / std::filesystem::path(name + ".epproj"));
	}

	auto EditorLayer::OpenProject() -> bool
	{
		const auto path = FileDialog::OpenFile({
			{ "EppoEngine Project", "epproj" }
		}, FS::GetRootDirectory());

		if (path.empty())
			return false;

		return OpenProject(path);
	}

	auto EditorLayer::OpenProject(const std::filesystem::path& path) -> bool
	{
		if (path.extension().string() != ".epproj")
		{
			Log::Error("Could not load '{}' because it is not a project file!", path);
			return false;
		}

		if (Project::GetActive())
			CloseProject();

		if (Project::Open(path))
		{
			const auto& projSpec = Project::GetActive()->GetSpecification();
			if (projSpec.StartScene)
				OpenScene(projSpec.StartScene);
			else
				NewScene();
		}

		return true;
	}

	auto EditorLayer::SaveProject() -> bool
	{
		SaveScene();

		if (!Project::GetActive()->GetSpecification().StartScene)
			Project::GetActive()->GetSpecification().StartScene = m_ActiveScene->Handle;

		return Project::SaveActive();
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
			m_ActiveScenePath = Project::GetAssetFilepath(path);
			m_PanelManager->SetSceneContext(m_ActiveScene);
		}
		else
		{
			Log::Error("Failed to deserialize scene '{}'!", path);
			return false;
		}

		return true;
	}

	auto EditorLayer::OpenScene(AssetHandle handle) -> void
	{
		const auto& assetManager = Project::GetActive()->GetAssetManager();
		m_EditorScene = std::static_pointer_cast<Scene>(assetManager->GetAsset(handle));
		m_ActiveScene = m_EditorScene;
		m_ActiveScenePath = Project::GetAssetFilepath(assetManager->GetMetadata(handle).Filepath);

		m_PanelManager->SetSceneContext(m_ActiveScene);
	}

	auto EditorLayer::SaveScene() -> bool
	{
		bool saved = false;

		if (m_ActiveScenePath.empty())
			saved = SaveSceneAs();
		else
		{
			const SceneSerializer serializer(m_ActiveScene);
			saved = serializer.Serialize(m_ActiveScenePath);
		}

		const auto& assetManager = Project::GetActive()->GetAssetManager();
		if (assetManager && !assetManager->HasAssetMetadata(m_ActiveScene->Handle))
			assetManager->CreateAsset(m_ActiveScenePath, m_ActiveScene);

		return saved;
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

	auto EditorLayer::UI_NewProjectPopup() -> void
	{
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
		if (ImGui::BeginPopupModal("New Project", nullptr, windowFlags))
		{
			static std::string projectName;

			ImGui::Text("Project Name");
			ImGui::InputText("##ProjectName", &projectName);

			const auto projectPath = FS::GetRootDirectory() / "Projects" / projectName;
			const bool projectExists = !projectName.empty() && FS::Exists(projectPath);

			if (projectExists)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
				ImGui::Text("Project name already exists!");
				ImGui::PopStyleColor();
			}
			else if (!projectName.empty())
			{
				ImGui::Text("Project path: \n%s", projectPath.string().c_str());
			}

			if (ImGui::Button("Cancel", ImVec2(100, 30)))
				ImGui::CloseCurrentPopup();

			ImGui::SameLine();

			if (projectExists)
				ImGui::BeginDisabled();

			if (ImGui::Button("Create", ImVec2(100, 30)))
			{
				NewProject(projectName);
				ImGui::CloseCurrentPopup();
			}

			if (projectExists)
				ImGui::EndDisabled();

			ImGui::EndPopup();
		}
	}
}