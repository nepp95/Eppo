#include "pch.h"
#include "UI/UILayer.h"

#include "Core/Application.h"

#include <RmlUi/Core.h>

namespace Eppo
{
	auto UILayer::OnAttach() -> void
	{
		m_RenderInterface = CreateScopedPtr<UIRenderInterface>();
		Rml::SetRenderInterface(m_RenderInterface.get());

		bool success = Rml::Initialise();
		EP_ASSERT(success);

		const auto& app = Application::Get();
		m_Context = Rml::CreateContext("default", Rml::Vector2i(app.GetWindow()->GetWidth(), app.GetWindow()->GetHeight()));
		EP_ASSERT(m_Context);

		auto fontPath = FS::GetResourcesDirectory() / "Fonts" / "Roboto-Regular.ttf";
		success = Rml::LoadFontFace(fontPath.string());
	}

	auto UILayer::OnDetach() -> void
	{
		Rml::Shutdown();
	}

	auto UILayer::OnUpdate(float timestep) -> void
	{
		m_Context->Update();
	}

	auto UILayer::OnUIRender() -> void
	{
		m_Context->Render();
	}

	auto UILayer::OnEvent(Event& e) -> void
	{
	}
}