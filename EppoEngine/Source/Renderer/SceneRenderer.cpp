#include "pch.h"
#include "Renderer/SceneRenderer.h"

#include "Core/Application.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Vertex.h"

#include <nvrhi/utils.h>

namespace Eppo
{
	SceneRenderer::SceneRenderer(const Ref<Scene>& scene, uint32_t width, uint32_t height)
		: m_Scene(scene), m_Width(width), m_Height(height)
	{
		const auto& dm = DeviceManager::Get();
		auto device = dm->GetDevice();
		const auto& renderer = dm->GetRenderer();

		m_CommandList = device->createCommandList();

		if (m_Width == 0 || m_Height == 0)
		{
			const auto& app = Application::Get();
			m_Width = app.GetWindow()->GetWidth();
			m_Height = app.GetWindow()->GetHeight();
		}

		m_VB = VertexBuffer::CreateCube();
		m_IB = IndexBuffer::CreateCube();

		// Geometry Pipeline
		{
			FramebufferSpecification framebufferSpec{
				.Width = m_Width,
				.Height = m_Height,
				.Attachments = { nvrhi::Format::RGBA8_UNORM },
				.ClearColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
				.ClearColorOnLoad = true,
				.DebugName = "Framebuffer Geometry",
			};
			
			PipelineSpecification pipelineSpec{
				.Shader = renderer->GetShader("geometry"),
				.Framebuffer = CreateRef<Framebuffer>(framebufferSpec),
				.Width = m_Width,
				.Height = m_Height,
				.DepthTestEnable = false,
				.DepthWriteEnable = false,
			};

			m_GeometryPipeline = CreateRef<Pipeline>(pipelineSpec);
		}

		// Uniform buffers
		m_CameraUB = CreateRef<UniformBuffer>(sizeof(CameraData), "UniformBuffer Camera");
		m_LightsUB = CreateRef<UniformBuffer>(sizeof(glm::vec4) * 4, "UniformBuffer Lights");
	
		constexpr float p = 15.0f;
		m_LightData[0] = glm::vec4(-p, -p * 0.5f, -p, 1.0f);
		m_LightData[1] = glm::vec4(-p, -p * 0.5f, p, 1.0f);
		m_LightData[2] = glm::vec4(p, -p * 0.5f, p, 1.0f);
		m_LightData[3] = glm::vec4(p, -p * 0.5f, -p, 1.0f);
		m_LightsUB->SetData(&m_LightData, sizeof(glm::vec4) * 4);
	}

	auto SceneRenderer::BeginScene(const ScopedPtr<EditorCamera>& camera) -> void
	{
		m_CameraData.View = camera->GetViewMatrix();
		m_CameraData.Projection = camera->GetProjectionMatrix();
		m_CameraData.ViewProjection = camera->GetViewProjection();
		m_CameraData.Position = glm::vec4(camera->GetPosition(), 0.0f);
		m_CameraUB->SetData(&m_CameraData, sizeof(CameraData));
	}

	auto SceneRenderer::EndScene() -> void
	{
		GeometryPass();
	}

	auto SceneRenderer::Resize(uint32_t width, uint32_t height) -> void
	{

	}

	auto SceneRenderer::GeometryPass() -> void
	{
		const auto& dm = DeviceManager::Get();
		auto device = dm->GetDevice();

		m_CommandList->open();

		// Clear framebuffer if needed
		const auto& framebuffer = m_GeometryPipeline->GetSpecification().Framebuffer;

		if (framebuffer->GetSpecification().ClearColorOnLoad)
		{
			const auto& clearColor = framebuffer->GetSpecification().ClearColor;
			for (uint32_t i = 0; i < framebuffer->GetFramebuffer()->getDesc().colorAttachments.size(); i++)
				nvrhi::utils::ClearColorAttachment(m_CommandList, framebuffer->GetFramebuffer(), i, nvrhi::Color(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
		}

		if (framebuffer->GetSpecification().ClearDepthOnLoad)
		{
			EP_ASSERT(false); // Test stencil index
			const auto& depthValue = framebuffer->GetSpecification().DepthClearValue;
			nvrhi::utils::ClearDepthStencilAttachment(m_CommandList, framebuffer->GetFramebuffer(), depthValue, 0);
		}

		// Setup graphics state
		nvrhi::GraphicsState state{
			.pipeline = m_GeometryPipeline->GetPipeline(),
			.framebuffer = framebuffer->GetFramebuffer(),
		};

		// Viewport and scissor
		state.viewport.viewports = { nvrhi::Viewport(static_cast<float>(m_Width), static_cast<float>(m_Height)) };
		state.viewport.scissorRects = { nvrhi::Rect(m_Width, m_Height) };

		// Vertex buffer
		nvrhi::VertexBufferBinding vtxBufBinding{
			.buffer = m_VB->GetBuffer(),
			.slot = 0,
			.offset = 0,
		};

		state.addVertexBuffer(vtxBufBinding);

		// Index buffer
		state.indexBuffer.buffer = m_IB->GetBuffer();
		state.indexBuffer.format = nvrhi::Format::R32_UINT;
		state.indexBuffer.offset = 0;

		// Binding sets
		struct PushConstants
		{
			glm::mat4 Transform;
			glm::vec3 MeshPosition;
			float Metallic;
			glm::vec3 Color;
			float Roughness;
		} pushConstants{};

		pushConstants.Transform = glm::mat4(1.0f);
		pushConstants.MeshPosition = glm::vec3(0.0f);

		nvrhi::BindingSetDesc desc{};
		desc.bindings = {
			nvrhi::BindingSetItem::PushConstants(0, sizeof(PushConstants)),
			nvrhi::BindingSetItem::ConstantBuffer(1, m_CameraUB->GetBuffer()),
			nvrhi::BindingSetItem::ConstantBuffer(2, m_LightsUB->GetBuffer())
		};

		const auto bindingLayout = m_GeometryPipeline->GetSpecification().Shader->GetBindingLayouts().at(0);
		const auto bindingSet = device->createBindingSet(desc, bindingLayout);
		state.addBindingSet(bindingSet);

		// Draw args
		nvrhi::DrawArguments drawArgs{
			.vertexCount = 24,
		};

		m_CommandList->setGraphicsState(state);
		m_CommandList->setPushConstants(&pushConstants, sizeof(PushConstants));
		m_CommandList->drawIndexed(drawArgs);
		m_CommandList->close();
		device->executeCommandList(m_CommandList);
	}
}