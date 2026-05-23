#include "pch.h"
#include "Renderer/Image.h"

#include "Renderer/DeviceManager.h"

namespace Eppo
{
	Image::Image(ImageSpecification spec)
		: m_Specification(std::move(spec))
	{
		const auto& dm = DeviceManager::Get();
		auto device = dm->GetDevice();

		nvrhi::TextureDesc textureDesc{
			.width = m_Specification.Width,
			.height = m_Specification.Height,
			.format = m_Specification.ImageFormat,
			.debugName = m_Specification.DebugName,
			.isRenderTarget = m_Specification.IsRenderTarget,
			.initialState = m_Specification.InitialState,
			.keepInitialState = m_Specification.AutomaticStateTracking,
		};

		if (m_Specification.ExistingImage)
		{
			if (dm->GetParams().API == RendererAPI::Vulkan)
				m_Texture = device->createHandleForNativeTexture(nvrhi::ObjectTypes::VK_Image, nvrhi::Object(m_Specification.ExistingImage), textureDesc);
			else
				EP_ASSERT(false);
		}
		else
		{
			m_Texture = device->createTexture(textureDesc);
		}
	}
}