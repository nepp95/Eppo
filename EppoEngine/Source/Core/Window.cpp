#include "pch.h"
#include "Core/Window.h"

#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"

#include <GLFW/glfw3.h>

namespace Eppo
{
	namespace
	{
		auto GLFWErrorCallback(int error, const char* description) -> void
		{
			Log::Error(LogSource::Glfw, "({}) {}", error, description);
		}
	}

	Window::Window(const uint32_t width, const uint32_t height)
		: m_Width(width), m_Height(height)
	{
		// Setup glfw
		const int success = glfwInit();
		EP_ASSERT(success, "GLFW failed to initialize!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
		glfwSetErrorCallback(GLFWErrorCallback);

		// Create window
		Log::Info("Creating window of size {}x{}", m_Width, m_Height);
		m_Window = glfwCreateWindow(static_cast<int>(m_Width), static_cast<int>(m_Height), "EppoEngine", nullptr, nullptr);

		// Set event callbacks
		glfwSetWindowUserPointer(m_Window, &m_EventCallback);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) -> void
			{
				const EventCallbackFn& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
				WindowCloseEvent e;
				callback(e);
			}
		);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, const int width, const int height) -> void
			{
				const EventCallbackFn& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
				WindowResizeEvent e(width, height);
				callback(e);
			}
		);

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, const int key, const int scancode, const int action, const int mods) -> void
			{
				const EventCallbackFn& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent e(key);
						callback(e);
						break;
					}

					case GLFW_RELEASE:
					{
						KeyReleasedEvent e(key);
						callback(e);
						break;
					}

					case GLFW_REPEAT:
					{
						KeyPressedEvent e(key, true);
						callback(e);
						break;
					}
				}
			}
		);

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, const unsigned int key) -> void
			{
				const EventCallbackFn& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
				KeyTypedEvent e(key);
				callback(e);
			}
		);
	}

	auto Window::Init() -> void
	{
		// Create device manager (dx11/dx12/vk)
		DeviceParams deviceParams{};

		m_DeviceManager = DeviceManager::Create(deviceParams);
		m_DeviceManager->Init();
	}

	auto Window::Shutdown() -> void
	{
		m_DeviceManager->Shutdown();

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	auto Window::ProcessEvents() -> void
	{
		glfwPollEvents();
	}
}