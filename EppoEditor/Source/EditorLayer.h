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
		std::shared_ptr<PanelManager> m_PanelManager = nullptr;

		std::shared_ptr<Scene> m_ActiveScene = nullptr;
		std::shared_ptr<Scene> m_EditorScene = nullptr;
		std::shared_ptr<SceneRenderer> m_SceneRenderer = nullptr;

		ScopedPtr<EditorCamera> m_EditorCamera = nullptr;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		enum class SceneState
		{
			Edit,
			Play,
		} m_SceneState = SceneState::Edit;
	};
}