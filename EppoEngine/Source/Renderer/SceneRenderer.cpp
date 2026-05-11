#include "pch.h"
#include "Renderer/SceneRenderer.h"

namespace Eppo
{
	SceneRenderer::SceneRenderer(const std::shared_ptr<Scene>& scene)
		: m_Scene(scene)
	{

	}

	auto SceneRenderer::BeginScene(const ScopedPtr<EditorCamera>& camera) -> void
	{
		m_CameraData.View = camera->GetViewMatrix();
		m_CameraData.Projection = camera->GetProjectionMatrix();
		m_CameraData.ViewProjection = camera->GetViewProjection();
		m_CameraData.Position = glm::vec4(camera->GetPosition(), 0.0f);
	}

	auto SceneRenderer::EndScene() -> void
	{

	}

	auto SceneRenderer::Resize(uint32_t width, uint32_t height) -> void
	{

	}
}