#include "pch.h"
#include "Renderer/Framebuffer.h"

#include "Core/Application.h"

namespace Eppo
{
	Framebuffer::Framebuffer(FramebufferSpecification spec)
		: m_Specification(std::move(spec))
	{
		const auto& app = Application::Get();
		const auto& dm = DeviceManager::Get();
		auto device = dm->GetDevice();

		if (m_Specification.Width == 0 || m_Specification.Height == 0)
		{
			// No size information, use window size
			m_Width = app.GetWindow()->GetWidth();
			m_Height = app.GetWindow()->GetHeight();
		}
		else
		{
			m_Width = m_Specification.Width;
			m_Height = m_Specification.Height;
		}

		// Create images if necessary
		nvrhi::FramebufferDesc framebufferDesc{};

		if (m_Specification.SwapchainTarget)
		{
			m_Images.emplace_back(m_Specification.SwapchainImage);
			framebufferDesc.addColorAttachment(m_Specification.SwapchainImage->GetTexture());
		}
		else
		{
			uint32_t attachmentIndex = 0;
			for (const auto& attachment : m_Specification.Attachments.Attachments)
			{
				ImageSpecification spec{
					.ImageFormat = attachment.ImageFormat,
					.Width = m_Width,
					.Height = m_Height,
					.DebugName = std::format("{} Image {}", m_Specification.DebugName, attachmentIndex),
				};

				std::shared_ptr<Image> image = std::make_shared<Image>(spec);
				m_Images.emplace_back(image);
				framebufferDesc.addColorAttachment(image->GetTexture());
				attachmentIndex++;
			}
		}

		m_Framebuffer = device->createFramebuffer(framebufferDesc);
	}
}