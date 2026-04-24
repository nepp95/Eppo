#pragma once

#include "Core/Layer.h"
#include "Core/Window.h"
#include "Event/ApplicationEvent.h"

auto main(int argc, char** argv) -> int;

namespace Eppo
{
	struct CommandLineArgs
	{
		int Argc;
		char** Argv;

		CommandLineArgs(int argc, char** argv)
			: Argc(argc), Argv(argv)
		{}

		auto operator[](const int index) const -> const char*
		{
			EP_ASSERT(index < Argc);
			if (index >= Argc)
				return "";
			return Argv[index];
		}
	};

	struct ApplicationParams
	{
		CommandLineArgs Args;

		uint32_t Width = 1600;
		uint32_t Height = 900;
		bool VSync = false;
	};

	class Application
	{
	public:
		Application(ApplicationParams&& params);
		virtual ~Application();

		auto Init() -> bool;
		auto Close() -> void { m_IsRunning = false; }

		auto Run() -> void;
		auto OnEvent(Event& e) -> void;

		template<typename T>
			requires(std::derived_from<T, Layer>)
		auto PushLayer() -> std::shared_ptr<T>
		{
			std::shared_ptr<T> layer = std::make_shared<T>();
			m_LayerStack.emplace_back(layer);
			layer->OnAttach();
			return layer;
		}

		[[nodiscard]] constexpr auto GetParams() const -> const ApplicationParams& { return m_Params; }
		[[nodiscard]] constexpr auto GetWindow() const -> const ScopedPtr<Window>& { return m_Window; }

		static auto Get() -> Application& { return *s_Instance; }

	protected:
		ApplicationParams m_Params;

	private:
		auto OnWindowClose(const WindowCloseEvent& e) -> bool;
		auto OnWindowResize(const WindowResizeEvent& e) -> bool;

	private:
		ScopedPtr<Window> m_Window = nullptr;
		std::vector<std::shared_ptr<Layer>> m_LayerStack;

		float m_LastFrameTime = 0.0f;
		bool m_IsRunning = true;
		bool m_IsMinimized = false;

		static Application* s_Instance;
		friend auto ::main(int argc, char** argv) -> int;
	};

	// IMPORTANT: To be implemented by the application!
	auto CreateApplication(int argc, char** argv) -> Application*;
}