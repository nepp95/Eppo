#pragma once

#include "Core/Layer.h"
#include "UI/UIRenderInterface.h"

#include <RmlUi/Core.h>

namespace Eppo
{
	class UILayer : public Layer
	{
	public:
		auto OnAttach() -> void override;
		auto OnDetach() -> void override;
		auto OnUpdate(float timestep) -> void override;
		auto OnUIRender() -> void override;
		auto OnEvent(Event& e) -> void override;

	private:
		Rml::Context* m_Context = nullptr;

		ScopedPtr<UIRenderInterface> m_RenderInterface = nullptr;
	};
}