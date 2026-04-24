#include "pch.h"
#include "Core/Application.h"

#include "Renderer/DeviceManager.h"
#include "UI/UILayer.h"

#include <GLFW/glfw3.h>
#include <nvrhi/utils.h>

namespace Eppo
{
	Application* Application::s_Instance = nullptr;

	Application::Application(ApplicationParams&& params)
		: m_Params(std::move(params))
	{
		EP_ASSERT(!s_Instance, "There can only be one instance of the application!");
		s_Instance = this;
	}

	Application::~Application()
	{
		Log::Info("Application shutting down...");

		for (auto it = m_LayerStack.begin(); it != m_LayerStack.end();)
		{
			std::shared_ptr layer = *it;
			layer->OnDetach();
			it = m_LayerStack.erase(it);
		}

		m_Window->Shutdown();
	}

	auto Application::Init() -> bool
	{
		// Create window
		m_Window = CreateScopedPtr<Window>(1600, 900);
		m_Window->Init();
		m_Window->SetEventCallback(
			[this](Event& e) -> void
			{
				OnEvent(e);
			}
		);

		// Create UI layer
		PushLayer<UILayer>();

		return true;
	}

	auto Application::Run() -> void
	{
		const auto& dm = DeviceManager::Get();

		while (m_IsRunning)
		{
			const auto time = static_cast<float>(glfwGetTime());
			const float timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_Window->ProcessEvents();

			if (dm->BeginFrame())
			{
				// Render work
				for (const auto& layer : m_LayerStack)
					layer->OnUpdate(timestep);

				const auto& dm = DeviceManager::Get();
				const auto device = dm->GetDevice();
				nvrhi::CommandListHandle cmdList = device->createCommandList();

				cmdList->open();
				nvrhi::utils::ClearColorAttachment(cmdList, dm->GetCurrentSwapchainImage().Framebuffer, 0, nvrhi::Color(0.3f));
				cmdList->close();

				device->executeCommandList(cmdList);

				// UI
				for (const auto& layer : m_LayerStack)
					layer->OnUIRender();

				// Do something UI Related here?

				// Present
				dm->Present();
			}

			dm->GetDevice()->runGarbageCollection();
		}

		dm->GetDevice()->waitForIdle();
	}

	auto Application::OnEvent(Event& e) -> void
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(std::bind_front(&Application::OnWindowClose, this));
		dispatcher.Dispatch<WindowResizeEvent>(std::bind_front(&Application::OnWindowResize, this));

		for (const auto& layer : m_LayerStack)
		{
			if (e.Handled)
				break;

			layer->OnEvent(e);
		}
	}

	auto Application::OnWindowClose(const WindowCloseEvent& e) -> bool
	{
		Close();

		return true;
	}

	auto Application::OnWindowResize(const WindowResizeEvent& e) -> bool
	{
		const uint32_t width = e.GetWidth();
		const uint32_t height = e.GetHeight();

		if (width == 0 || height == 0)
		{
			m_IsMinimized = true;
			return false;
		}

		m_IsMinimized = false;

		return true;
	}
}