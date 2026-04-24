#include "EditorLayer.h"

namespace Eppo
{
	EditorLayer::EditorLayer()
	{
		m_ActiveScene = std::make_shared<Scene>();
		m_ActiveScene->CreateEntity("Test");
	}

	EditorLayer::~EditorLayer()
	{

	}

	auto EditorLayer::OnAttach() -> void
	{
	}

	auto EditorLayer::OnDetach() -> void
	{
	}

	auto EditorLayer::OnUpdate(float timestep) -> void
	{

	}

	auto EditorLayer::OnUIRender() -> void
	{

	}

	auto EditorLayer::OnEvent(Event& e) -> void
	{
	}
}