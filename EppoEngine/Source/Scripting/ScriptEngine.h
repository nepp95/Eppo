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

		std::wstring m_AppAssemblyPath;

		using BootstrapFn = void(*)();
		using GetCountFn = int(*)();
		using GetNameFn = char* (*)(int);
		using FreeStringFn = void(*)(char*);
		using InvokeFn = void(*)(const char*);
		BootstrapFn m_BootstrapFn = nullptr;
		GetCountFn m_GetCountFn = nullptr;
		GetNameFn m_GetNameFn = nullptr;
		FreeStringFn m_FreeStringFn = nullptr;
		InvokeFn m_InvokeFn = nullptr;
	};
}