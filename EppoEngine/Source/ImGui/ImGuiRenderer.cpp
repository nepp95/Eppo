#include "pch.h"
#include "ImGui/ImGuiRenderer.h"

#include "Renderer/DeviceManager.h"

// TODO: TEMPORARY
#include "Platform/Vulkan/Swapchain.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <nvrhi/utils.h>

namespace Eppo
{
	ImGuiRenderer::ImGuiRenderer()
	{
		const auto& dm = DeviceManager::Get();
		const auto& renderer = dm->GetRenderer();
		const auto device = dm->GetDevice();

		auto& io = ImGui::GetIO();
		io.BackendRendererName = "ImGuiRenderer";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

		// Input layout
		const std::array<nvrhi::VertexAttributeDesc, 3> vertexAttributeDescs = {
			nvrhi::VertexAttributeDesc{
				.name = "POSITION",
				.format = nvrhi::Format::RG32_FLOAT,
				.arraySize = 1,
				.bufferIndex = 0,
				.offset = offsetof(ImDrawVert, pos),
				.elementStride = sizeof(ImDrawVert),
				.isInstanced = false,
			},
			nvrhi::VertexAttributeDesc{
				.name = "TEXCOORD",
				.format = nvrhi::Format::RG32_FLOAT,
				.arraySize = 1,
				.bufferIndex = 0,
				.offset = offsetof(ImDrawVert, uv),
				.elementStride = sizeof(ImDrawVert),
				.isInstanced = false,
			},
			nvrhi::VertexAttributeDesc{
				.name = "COLOR",
				.format = nvrhi::Format::RGBA8_UNORM,
				.arraySize = 1,
				.bufferIndex = 0,
				.offset = offsetof(ImDrawVert, col),
				.elementStride = sizeof(ImDrawVert),
				.isInstanced = false,
			},
		};

		const auto& shader = renderer->GetShader("imgui");

		nvrhi::InputLayoutHandle inputLayout = device->createInputLayout(vertexAttributeDescs.data(), static_cast<uint32_t>(vertexAttributeDescs.size()), shader->GetVertexShaderHandle());

		// Render state
		nvrhi::BlendState blendState;
		blendState.targets[0].blendEnable = true;
		blendState.targets[0].srcBlend = nvrhi::BlendFactor::SrcAlpha;
		blendState.targets[0].destBlend = nvrhi::BlendFactor::InvSrcAlpha;
		blendState.targets[0].srcBlendAlpha = nvrhi::BlendFactor::One;
		blendState.targets[0].destBlendAlpha = nvrhi::BlendFactor::InvSrcAlpha;

		nvrhi::RasterState rasterState{
			.fillMode = nvrhi::RasterFillMode::Solid,
			.cullMode = nvrhi::RasterCullMode::None,
			.depthClipEnable = true,
			.scissorEnable = true,
		};

		nvrhi::DepthStencilState depthStencilState{
			.depthTestEnable = false,
			.depthWriteEnable = true,
			.depthFunc = nvrhi::ComparisonFunc::Always,
			.stencilEnable = false,
		};

		nvrhi::RenderState renderState{
			.blendState = blendState,
			.depthStencilState = depthStencilState,
			.rasterState = rasterState,
		};

		// Binding layout
		nvrhi::BindingLayoutDesc bindingLayoutDesc{
			.visibility = nvrhi::ShaderType::All,
			.bindings = {
				nvrhi::BindingLayoutItem::PushConstants(0, sizeof(glm::vec2) * 2),
				nvrhi::BindingLayoutItem::Texture_SRV(0),
				nvrhi::BindingLayoutItem::Sampler(0)
			},
		};

		m_BindingSetLayout = device->createBindingLayout(bindingLayoutDesc);

		// Pipeline
		m_PipelineDesc.inputLayout = inputLayout;
		m_PipelineDesc.VS = shader->GetVertexShaderHandle();
		m_PipelineDesc.PS = shader->GetPixelShaderHandle();
		m_PipelineDesc.renderState = renderState;
		m_PipelineDesc.addBindingLayout(m_BindingSetLayout);

		// Sampler
		nvrhi::SamplerDesc samplerDesc{};
		samplerDesc.setAllAddressModes(nvrhi::SamplerAddressMode::Wrap);
		samplerDesc.setAllFilters(true);

		m_FontSampler = device->createSampler(samplerDesc);
		EP_ASSERT(m_FontSampler);

		// Command list
		m_CommandList = device->createCommandList();
	}

	auto ImGuiRenderer::Resize() -> void
	{
		m_PipelineCache.clear();
	}

	auto ImGuiRenderer::UpdateFontTexture() -> void
	{
		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();
		auto& io = ImGui::GetIO();

		if (m_FontTexture && io.Fonts->TexRef.GetTexID())
			return;

		unsigned char* pixels;
		int width, height;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		EP_ASSERT(pixels);

		nvrhi::TextureDesc textureDesc{
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height),
			.format = nvrhi::Format::RGBA8_UNORM,
			.debugName = "ImGui Font Texture",
		};

		m_FontTexture = device->createTexture(textureDesc);
		EP_ASSERT(m_FontTexture);

		m_CommandList->open();
		m_CommandList->beginTrackingTextureState(m_FontTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common);
		m_CommandList->writeTexture(m_FontTexture, 0, 0, pixels, static_cast<size_t>(width) * 4);
		m_CommandList->setPermanentTextureState(m_FontTexture, nvrhi::ResourceStates::ShaderResource);
		m_CommandList->commitBarriers();
		m_CommandList->close();

		device->executeCommandList(m_CommandList);
		io.Fonts->TexRef = ImTextureRef(m_FontTexture.Get());
	}

	auto ImGuiRenderer::RenderToSwapchain(ImGuiViewport* viewport, const ScopedPtr<Swapchain>& swapchain) -> void
	{
		Render(viewport, GetOrCreatePipeline(swapchain), swapchain->GetCurrentSwapchainImage().Framebuffer);
	}

	auto ImGuiRenderer::Render(ImGuiViewport* viewport, nvrhi::GraphicsPipelineHandle pipeline, nvrhi::FramebufferHandle framebuffer) -> void
	{
		const auto& dm = DeviceManager::Get();
		const auto& io = ImGui::GetIO();

		m_CommandList->open();

		const std::string marker = std::format("ImGui (Viewport: {})", viewport == ImGui::GetMainViewport() ? "Main" : std::to_string(reinterpret_cast<uint64_t>(viewport)));
		m_CommandList->beginMarker(marker.c_str());

		nvrhi::utils::ClearColorAttachment(m_CommandList, framebuffer, 0, nvrhi::Color(1, 0, 0, 1));

		// Update geometry
		ImDrawData* drawData = viewport->DrawData;
		UpdateGeometry(drawData);

		// DPI Scaling
		drawData->ScaleClipRects(drawData->FramebufferScale);

		// Setup graphics state
		struct PushConstants
		{
			glm::vec2 Scale;
			glm::vec2 Translate;
		} pushConstants{};

		pushConstants.Scale.x = 2.0f / drawData->DisplaySize.x;
		pushConstants.Scale.y = 2.0f / drawData->DisplaySize.y;
		pushConstants.Translate.x = -1.0f - drawData->DisplayPos.x * pushConstants.Scale.x;
		pushConstants.Translate.y = -1.0f - drawData->DisplayPos.y * pushConstants.Scale.y;

		float fbWidth = drawData->DisplaySize.x * drawData->FramebufferScale.x;
		float fbHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;

		nvrhi::GraphicsState state{
			.pipeline = pipeline,
			.framebuffer = framebuffer,
		};

		state.viewport.viewports.push_back(nvrhi::Viewport(fbWidth, fbHeight));
		state.viewport.scissorRects.resize(1);

		// Vertex buffer
		nvrhi::VertexBufferBinding vtxBufBinding{
			.buffer = m_VertexBuffer,
			.slot = 0,
			.offset = 0,
		};

		state.addVertexBuffer(vtxBufBinding);

		// Index buffer
		state.indexBuffer.buffer = m_IndexBuffer;
		state.indexBuffer.format = (sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT : nvrhi::Format::R32_UINT);
		state.indexBuffer.offset = 0;

		// Draw calls
		ImVec2 clipOffset = drawData->DisplayPos;
		ImVec2 clipScale = drawData->FramebufferScale;
		uint32_t vtxOffset = 0;
		uint32_t idxOffset = 0;

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* drawList = drawData->CmdLists[n];

			for (int i = 0; i < drawList->CmdBuffer.Size; i++)
			{
				const ImDrawCmd* drawCmd = &drawList->CmdBuffer[i];

				if (drawCmd->UserCallback)
				{
					drawCmd->UserCallback(drawList, drawCmd);
				}
				else
				{
					state.bindings = { GetOrCreateBindingSet((nvrhi::ITexture*)drawCmd->TexRef.GetTexID()) };
					EP_ASSERT(state.bindings[0]);

					ImVec2 clipMin((drawCmd->ClipRect.x - clipOffset.x) * clipScale.x, (drawCmd->ClipRect.y - clipOffset.y) * clipScale.y);
					ImVec2 clipMax((drawCmd->ClipRect.z - clipOffset.x) * clipScale.x, (drawCmd->ClipRect.w - clipOffset.y) * clipScale.y);

					if (clipMin.x < 0.0f)
						clipMin.x = 0.0f;
					if (clipMin.y < 0.0f)
						clipMin.y = 0.0f;
					if (clipMax.x > fbWidth)
						clipMax.x = fbWidth;
					if (clipMax.y > fbHeight)
						clipMax.y = fbHeight;
					if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
						continue;

					state.viewport.scissorRects[0] = nvrhi::Rect(static_cast<int>(clipMin.x), static_cast<int>(clipMax.x), static_cast<int>(clipMin.y), static_cast<int>(clipMax.y));

					nvrhi::DrawArguments drawArgs{
						.vertexCount = drawCmd->ElemCount,
						.startIndexLocation = drawCmd->IdxOffset + idxOffset,
						.startVertexLocation = drawCmd->VtxOffset + vtxOffset,
					};

					m_CommandList->setGraphicsState(state);
					m_CommandList->setPushConstants(&pushConstants, sizeof(PushConstants));
					m_CommandList->drawIndexed(drawArgs);
				}
			}
			idxOffset += drawList->IdxBuffer.Size;
			vtxOffset += drawList->VtxBuffer.Size;
		}

		m_CommandList->endMarker();
		m_CommandList->close();

		dm->GetDevice()->executeCommandList(m_CommandList);
	}

	auto ImGuiRenderer::UpdateGeometry(ImDrawData* drawData) -> void
	{
		// Calculate buffer size + a small margin since we can often have the case we just need a little bit more (menu's, hover effects, etc...)
		uint64_t requiredVtxSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
		uint64_t reallocateVtxSize = UINT64_MAX - requiredVtxSize > 1024 ? requiredVtxSize + 1024 : UINT64_MAX;
		if (!m_VertexBuffer || m_VertexBuffer->getDesc().byteSize < requiredVtxSize)
			m_VertexBuffer = ReallocateBuffer(reallocateVtxSize, false);

		uint64_t requiredIdxSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
		uint64_t reallocateIdxSize = UINT64_MAX - requiredIdxSize > 1024 ? requiredIdxSize + 1024 : UINT64_MAX;
		if (!m_IndexBuffer || m_IndexBuffer->getDesc().byteSize < requiredIdxSize)
			m_IndexBuffer = ReallocateBuffer(reallocateIdxSize, true);

		// Update local data
		m_LocalVertexData.resize(drawData->TotalVtxCount);
		m_LocalIndexData.resize(drawData->TotalIdxCount);

		ImDrawVert* vtxDst = &m_LocalVertexData[0];
		ImDrawIdx* idxDst = &m_LocalIndexData[0];

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* drawList = drawData->CmdLists[n];

			std::memcpy(vtxDst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
			std::memcpy(idxDst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));

			vtxDst += drawList->VtxBuffer.Size;
			idxDst += drawList->IdxBuffer.Size;
		}

		m_CommandList->writeBuffer(m_VertexBuffer, m_LocalVertexData.data(), m_LocalVertexData.size() * sizeof(ImDrawVert));
		m_CommandList->writeBuffer(m_IndexBuffer, m_LocalIndexData.data(), m_LocalIndexData.size() * sizeof(ImDrawIdx));
	}

	auto ImGuiRenderer::ReallocateBuffer(uint64_t size, bool indexBuffer) -> nvrhi::BufferHandle
	{
		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();

		nvrhi::BufferDesc bufferDesc{
			.byteSize = size,
			.debugName = indexBuffer ? "ImGui IndexBuffer" : "ImGui VertexBuffer",
			.isVertexBuffer = !indexBuffer ? true : false,
			.isIndexBuffer = indexBuffer ? true : false,
			.initialState = indexBuffer ? nvrhi::ResourceStates::IndexBuffer : nvrhi::ResourceStates::VertexBuffer,
			.keepInitialState = true,
		};

		return device->createBuffer(bufferDesc);
	}

	auto ImGuiRenderer::GetOrCreatePipeline(const ScopedPtr<Swapchain>& swapchain) -> nvrhi::GraphicsPipelineHandle
	{
		uint32_t framebufferIndex = swapchain->GetCurrentBackBufferIndex();
		auto& pipelineCache = m_PipelineCache[swapchain.get()];

		nvrhi::FramebufferHandle targetFramebuffer = swapchain->GetCurrentSwapchainImage().Framebuffer;
		nvrhi::GraphicsPipelineHandle pipeline = pipelineCache.Pipelines.at(framebufferIndex);

		bool invalidate = !pipeline || pipelineCache.Framebuffers.at(framebufferIndex) != targetFramebuffer;
		if (invalidate)
		{
			const auto& dm = DeviceManager::Get();
			const auto device = dm->GetDevice();
			
			pipeline = device->createGraphicsPipeline(m_PipelineDesc, targetFramebuffer);
			pipelineCache.Pipelines.at(framebufferIndex) = pipeline;
			pipelineCache.Framebuffers.at(framebufferIndex) = targetFramebuffer;
		}

		return pipeline;
	}

	auto ImGuiRenderer::GetOrCreateBindingSet(nvrhi::TextureHandle texture) -> nvrhi::BindingSetHandle
	{
		auto it = m_BindingSetCache.find(texture);
		if (it != m_BindingSetCache.end())
			return it->second;

		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		nvrhi::BindingSetDesc desc{};
		desc.bindings = {
			nvrhi::BindingSetItem::PushConstants(0, sizeof(float) * 2),
			nvrhi::BindingSetItem::Texture_SRV(0, texture),
			nvrhi::BindingSetItem::Sampler(0, m_FontSampler)
		};

		nvrhi::BindingSetHandle bindingSet;
		bindingSet = device->createBindingSet(desc, m_BindingSetLayout);
		EP_ASSERT(bindingSet);

		m_BindingSetCache[texture] = bindingSet;
		return bindingSet;
	}
}