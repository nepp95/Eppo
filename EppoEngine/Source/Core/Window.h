#pragma once

#include "Event/Event.h"
#include "Renderer/DeviceManager.h"

struct GLFWwindow;

namespace Eppo
{
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(uint32_t width, uint32_t height);
		~Window() = default;

		auto Init() -> void;
		auto Shutdown() -> void;

		auto ProcessEvents() -> void;
		auto SetEventCallback(const EventCallbackFn& callback) { m_EventCallback = callback; }

		[[nodiscard]] auto GetDeviceManager() -> std::shared_ptr<DeviceManager> { return m_DeviceManager; }

		[[nodiscard]] auto GetNative() const -> GLFWwindow* { return m_Window; }
		[[nodiscard]] auto GetWidth() const -> uint32_t { return m_Width; }
		[[nodiscard]] auto GetHeight() const -> uint32_t { return m_Height; }

	private:
		std::shared_ptr<DeviceManager> m_DeviceManager = nullptr;

		GLFWwindow* m_Window = nullptr;
		EventCallbackFn m_EventCallback;

		uint32_t m_Width;
		uint32_t m_Height;
	};
}