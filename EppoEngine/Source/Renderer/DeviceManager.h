#pragma once

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	enum class RendererAPI
	{
		None,
		DX11,
		DX12,
		Vulkan,
	};

	struct NvrhiMessageCallback : nvrhi::IMessageCallback
	{
		auto message(const nvrhi::MessageSeverity severity, const char* messageText) -> void override
		{
			switch (severity)
			{
				case nvrhi::MessageSeverity::Info:
				{
					Log::Info("{}", messageText);
					break;
				}

				case nvrhi::MessageSeverity::Warning:
				{
					Log::Warn("{}", messageText);
					break;
				}

				case nvrhi::MessageSeverity::Error:
				{
					Log::Error("{}", messageText);
					break;
				}

				case nvrhi::MessageSeverity::Fatal:
				{
					Log::Error("{}", messageText);
					EP_ASSERT(false, messageText);
					break;
				}
			}
		}
	};

	struct SwapchainImage
	{
		void* NativeImage = nullptr;
		nvrhi::TextureHandle Texture = nullptr;
		nvrhi::FramebufferHandle Framebuffer = nullptr;
	};

	struct DeviceParams
	{
		RendererAPI API = RendererAPI::Vulkan;
		uint32_t Width = 1600;
		uint32_t Height = 900;
	};

	class DeviceManager
	{
	public:
		virtual ~DeviceManager() = default;

		virtual auto Init() -> void = 0;
		virtual auto Shutdown() -> void = 0;

		virtual auto GetDevice() const -> nvrhi::IDevice* = 0;

		virtual auto BeginFrame() -> bool = 0;
		virtual auto Present() -> bool = 0;

		virtual auto GetCurrentBackBufferIndex() const -> uint32_t = 0;
		virtual auto GetCurrentSwapchainImage() -> const SwapchainImage& = 0;

		static auto Get() -> std::shared_ptr<DeviceManager>;
		static auto Create(const DeviceParams& params) -> std::shared_ptr<DeviceManager>;

	protected:
		DeviceManager(const DeviceParams& params);

	protected:
		DeviceParams m_Params;

		nvrhi::IMessageCallback* m_MessageCallback = nullptr;
	};
}