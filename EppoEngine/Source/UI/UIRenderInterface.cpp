#include "pch.h"
#include "UI/UIRenderInterface.h"

#include "Core/Application.h"
#include "Renderer/DeviceManager.h"

namespace Eppo
{
	Rml::CompiledGeometryHandle UIRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
	{
		m_VertexBuffer = CreateScopedPtr<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Rml::Vertex));
		m_IndexBuffer = CreateScopedPtr<IndexBuffer>(indices.data(), indices.size() * sizeof(int));

		return Rml::CompiledGeometryHandle(m_VertexBuffer.get());
	}

	void UIRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
	{
		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();
		const nvrhi::CommandListHandle cmdList = device->createCommandList();
		const auto& app = Application::Get();

		cmdList->open();
		//Maybe record to temp texture? When does this get called?
		// Binding sets todo

		auto graphicsState = nvrhi::GraphicsState{
			.pipeline = nullptr,
			.framebuffer = nullptr,
			.viewport = nvrhi::ViewportState().addViewportAndScissorRect(nvrhi::Viewport(app.GetWindow()->GetWidth(), app.GetWindow()->GetHeight())),
			.indexBuffer = m_IndexBuffer->GetBuffer(),
		};

		graphicsState.addVertexBuffer({
			.buffer = m_VertexBuffer->GetBuffer(),
			.slot = 0,
			.offset = 0
		});

		constexpr nvrhi::DrawArguments drawArgs{
			.vertexCount = 0,
			.instanceCount = 1,
		};

		cmdList->setGraphicsState(graphicsState);
		cmdList->drawIndexed(drawArgs);
		cmdList->close();

		device->executeCommandList(cmdList);
	}

	void UIRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
	{
		m_VertexBuffer.reset();
		m_IndexBuffer.reset();
	}

	Rml::TextureHandle UIRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	Rml::TextureHandle UIRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	void UIRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
	{
	}

	void UIRenderInterface::EnableScissorRegion(bool enable)
	{
	}

	void UIRenderInterface::SetScissorRegion(Rml::Rectanglei region)
	{
	}
}