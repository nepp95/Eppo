#include "pch.h"
#include "Core/Log.h"

namespace Eppo
{
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Log::s_FileLoggerSink = nullptr;
	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Log::s_ConsoleLoggerSink = nullptr;
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger = nullptr;
	std::shared_ptr<spdlog::logger> Log::s_GlfwLogger = nullptr;
	std::shared_ptr<spdlog::logger> Log::s_VulkanLogger = nullptr;

	auto Log::Init() -> void
	{
		if (std::filesystem::exists("latest.log"))
		{
			if (std::filesystem::exists("previous.log"))
				std::filesystem::remove("previous.log");
			std::filesystem::rename("latest.log", "previous.log");
		}

		s_FileLoggerSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("latest.log", true);
		s_FileLoggerSink->set_level(spdlog::level::trace);
		s_ConsoleLoggerSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		s_ConsoleLoggerSink->set_level(spdlog::level::trace);

		const spdlog::sinks_init_list sinks = { s_FileLoggerSink, s_ConsoleLoggerSink };

		s_CoreLogger = std::make_shared<spdlog::logger>("Core", sinks);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_GlfwLogger = std::make_shared<spdlog::logger>("Glfw", sinks);
		s_GlfwLogger->set_level(spdlog::level::trace);
		s_VulkanLogger = std::make_shared<spdlog::logger>("Vulkan", sinks);
		s_VulkanLogger->set_level(spdlog::level::trace);

		spdlog::set_default_logger(s_CoreLogger);
		s_CoreLogger->set_pattern("%^[%T.%e] [%n]: %v%$");
	}
}