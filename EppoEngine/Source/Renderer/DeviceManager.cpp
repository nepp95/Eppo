#include "pch.h"
#include "Renderer/DeviceManager.h"

#include "Core/Application.h"
#include "Platform/Vulkan/DeviceManagerVK.h"

namespace Eppo
{
	auto DeviceManager::Get() -> std::shared_ptr<DeviceManager>
	{
		return Application::Get().GetWindow()->GetDeviceManager();
	}

	auto DeviceManager::Create(const DeviceParams& params) -> std::shared_ptr<DeviceManager>
	{
		EP_ASSERT(params.API != RendererAPI::None, "No renderer api selected!");

		switch (params.API)
		{
			#if defined(EP_PLATFORM_WINDOWS)
			case RendererAPI::DX11:
			{
				EP_ASSERT(false, "Currently we do not support DX11!");
				break;
			}

			case RendererAPI::DX12:
			{
				EP_ASSERT(false, "Currently we do not support DX12!");
				break;
			}
			#endif

			case RendererAPI::Vulkan:
				return std::make_shared<DeviceManagerVK>(params);
		}

		EP_ASSERT(false);
	}

	DeviceManager::DeviceManager(const DeviceParams& params)
		: m_Params(params)
	{
		m_MessageCallback = new NvrhiMessageCallback();
	}
}