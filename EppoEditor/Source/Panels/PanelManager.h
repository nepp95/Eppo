#pragma once

#include "Panels/Panel.h"

#include <EppoEngine.h>

namespace Eppo
{
	struct PanelData
	{
		std::shared_ptr<Panel> Panel = nullptr;
		bool IsOpen = false;
	};

	class PanelManager
	{
	public:
		auto RenderGui() -> void;

		template<typename T>
			requires(std::derived_from<T, Panel>)
		auto AddPanel(const std::string& panelName, bool isOpen) -> void
		{
			PanelData data{
				.Panel = std::make_shared<T>(),
				.IsOpen = isOpen,
			};

			m_PanelData[panelName] = data;
		}

		template<typename T>
			requires(std::derived_from<T, Panel>)
		auto GetPanel(const std::string& panelName) -> std::shared_ptr<T>
		{
			auto it = m_PanelData.find(panelName);

			if (it != m_PanelData.end())
				return it->second;

			return nullptr;
		}

		auto HasPanel(const std::string& panelName) const -> bool { return m_PanelData.contains(panelName); }
		auto TogglePanel(const std::string& panelName) -> void;

	private:
		std::unordered_map<std::string, PanelData> m_PanelData;
		std::shared_ptr<Scene> m_SceneContext = nullptr;
	};
}