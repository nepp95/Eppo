#pragma once

#include "Panels/PanelManager.h"

#include <EppoEngine.h>

namespace Eppo
{
	class EditorLayer : public Layer
	{
	public:
		auto OnAttach() -> void override;
		auto OnDetach() -> void override;

		auto OnUpdate(float timestep) -> void override;
		auto OnUIRender() -> void override;
		auto OnEvent(Event& e) -> void override;
		
	private:
		Ref<PanelManager> m_PanelManager = nullptr;

		Ref<Scene> m_ActiveScene = nullptr;
		Ref<Scene> m_EditorScene = nullptr;
		Ref<SceneRenderer> m_SceneRenderer = nullptr;

		ScopedPtr<EditorCamera> m_EditorCamera = nullptr;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		uint32_t m_ViewportWidth = 1600;
		uint32_t m_ViewportHeight = 900;

		enum class SceneState
		{
			Edit,
			Play,
		} m_SceneState = SceneState::Edit;
	};
}