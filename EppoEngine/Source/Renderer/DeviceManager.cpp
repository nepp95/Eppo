#include "pch.h"
#include "Renderer/DeviceManager.h"

#include "Core/Application.h"
#include "Platform/Vulkan/DeviceManagerVK.h"

namespace Eppo
{
	auto DeviceManager::Get() -> std::shared_ptr<DeviceManager>
	{
		return Application::Get().GetDeviceManager();
	}

	auto DeviceManager::Create(const std::shared_ptr<Window>& window, const DeviceParams& params) -> ScopedPtr<DeviceManager>
	{
		EP_ASSERT(params.API != RendererAPI::None, "No renderer api selected!");
		#if !defined(EP_PLATFORM_WINDOWS)
		EP_ASSERT(params.API != RendererAPI::DX11, "DX11 renderer api selected on a non windows target!");
		EP_ASSERT(params.API != RendererAPI::DX12, "DX12 renderer api selected on a non windows target!");
		#endif

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
				return CreateScopedPtr<DeviceManagerVK>(window, params);
		}

		EP_ASSERT(false);
		return nullptr;
	}

	auto DeviceManager::WaitIdle() const -> bool
	{
		return GetDevice()->waitForIdle();
	}

	auto DeviceManager::InitRenderer() -> void
	{
		m_Renderer = CreateScopedPtr<Renderer>();
	}

	DeviceManager::DeviceManager(const std::shared_ptr<Window>& window, const DeviceParams& params)
		: m_Window(window), m_Params(params)
	{}
}