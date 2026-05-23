#pragma once

#include "Renderer/Image.h"

#include <glm/glm.hpp>
#include <nvrhi/nvrhi.h>

namespace Eppo
{
	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(const nvrhi::Format format)
			: ImageFormat(format)
		{}

		nvrhi::Format ImageFormat = nvrhi::Format::UNKNOWN;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments(attachments)
		{}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;

		FramebufferAttachmentSpecification Attachments;
		
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		float DepthClearValue = 0.0f;
		bool ClearColorOnLoad = true;
		bool ClearDepthOnLoad = true;

		bool SwapchainTarget = false;
		std::shared_ptr<Image> SwapchainImage = nullptr;

		std::string DebugName;
	};

	class Framebuffer
	{
	public:
		Framebuffer(FramebufferSpecification spec);

		[[nodiscard]] auto GetFramebuffer() const -> nvrhi::FramebufferHandle { return m_Framebuffer; }
		[[nodiscard]] constexpr auto GetSpecification() const -> const FramebufferSpecification& { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;
		nvrhi::FramebufferHandle m_Framebuffer = nullptr;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		std::vector<std::shared_ptr<Image>> m_Images;
	};
}