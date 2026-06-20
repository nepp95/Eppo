#pragma once

namespace Eppo
{
	class ScriptEngine
	{
	public:
		auto Init(const std::wstring& runtimeConfigPath) -> bool;
		auto Shutdown() -> void;

		auto LoadAssembly(const std::wstring& path) -> void*;

	private:
		auto GetManagedFnPointer(const std::wstring& assemblyPath, const std::wstring& typeName, const std::wstring& methodName) -> void*;

	private:
		void* m_HostFxrLib = nullptr;
		void* m_HostContext = nullptr;
		void* m_AppAssembly = nullptr;
	};
}