#pragma once

#include "Renderer/Framebuffer.h"
#include "Renderer/Shader.h"

namespace Eppo
{
	struct PipelineSpecification
	{
		std::shared_ptr<Shader> Shader;
		std::shared_ptr<Framebuffer> Framebuffer;

		uint32_t Width = 0;
		uint32_t Height = 0;

		// Rasterization
		nvrhi::RasterCullMode CullMode = nvrhi::RasterCullMode::Back;
		nvrhi::RasterFillMode FillMode = nvrhi::RasterFillMode::Solid;

		// Depth Stencil
		bool DepthTestEnable = true;
		bool DepthWriteEnable = true;
		nvrhi::ComparisonFunc DepthFunc = nvrhi::ComparisonFunc::Less;
	};

	class Pipeline
	{
	public:
		Pipeline(PipelineSpecification spec);

		[[nodiscard]] constexpr auto GetSpecification() const -> const PipelineSpecification& { return m_Specification; }
		[[nodiscard]] auto GetPipeline() const -> nvrhi::GraphicsPipelineHandle { return m_PipelineHandle; }

	private:
		PipelineSpecification m_Specification;

		nvrhi::GraphicsPipelineHandle m_PipelineHandle = nullptr;
	};
}