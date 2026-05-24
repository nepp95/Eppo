#include "pch.h"
#include "Renderer/SceneRenderer.h"

#include "Core/Application.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Renderer.h"

namespace Eppo
{
	SceneRenderer::SceneRenderer(const std::shared_ptr<Scene>& scene, uint32_t width, uint32_t height)
		: m_Scene(scene), m_Width(width), m_Height(height)
	{
		const auto& dm = DeviceManager::Get();
		const auto& renderer = dm->GetRenderer();

		if (m_Width == 0 || m_Height == 0)
		{
			const auto& app = Application::Get();
			m_Width = app.GetWindow()->GetWidth();
			m_Height = app.GetWindow()->GetHeight();
		}

		// Geometry Pipeline
		{
			FramebufferSpecification framebufferSpec{
				.Width = m_Width,
				.Height = m_Height,
				.Attachments = { nvrhi::Format::RGBA8_UNORM },
				.DebugName = "Framebuffer Geometry",
			};
			
			PipelineSpecification pipelineSpec{
				.Shader = renderer->GetShader("geometry"),
				.Framebuffer = std::make_shared<Framebuffer>(framebufferSpec),
				.Width = m_Width,
				.Height = m_Height,
				.DepthTestEnable = false,
				.DepthWriteEnable = false,
			};

			m_GeometryPipeline = std::make_shared<Pipeline>(pipelineSpec);
		}
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

	auto SceneRenderer::GeometryPass() -> void
	{

	}
}