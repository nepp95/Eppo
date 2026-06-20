#include "pch.h"
#include "Scripting/ScriptEngine.h"

#include "Scripting/Platform.h"

#include <coreclr_delegates.h>
#include <nethost.h>
#include <hostfxr.h>

namespace Eppo
{
	namespace
	{
		hostfxr_initialize_for_runtime_config_fn s_InitFn;
		hostfxr_get_runtime_delegate_fn s_GetDelegateFn;
		hostfxr_close_fn s_CloseFn;
	}

	auto ScriptEngine::Init(const std::wstring& runtimeConfigPath) -> bool
	{
		char_t hostFxrPath[MAX_PATH];
		size_t bufferSize = sizeof(hostFxrPath) / sizeof(char_t);

		if (get_hostfxr_path(hostFxrPath, &bufferSize, nullptr) != 0)
		{
			Log::Error("Could not locate hostfxr!");
			return false;
		}

		m_HostFxrLib = LOAD_LIB(hostFxrPath);
		EP_ASSERT(m_HostFxrLib);

		s_InitFn = (hostfxr_initialize_for_runtime_config_fn)GET_SYM(m_HostFxrLib, "hostfxr_initialize_for_runtime_config");
		s_GetDelegateFn = (hostfxr_get_runtime_delegate_fn)GET_SYM(m_HostFxrLib, "hostfxr_get_runtime_delegate");
		s_CloseFn = (hostfxr_close_fn)GET_SYM(m_HostFxrLib, "hostfxr_close");

		int result = s_InitFn(runtimeConfigPath.c_str(), nullptr, &m_HostContext);
		if (result != 0 || m_HostContext == nullptr)
		{
			Log::Error("hostfxr_initialize_for_runtime_config failed: {}", result);
			return false;
		}

		return true;
	}

	auto ScriptEngine::Shutdown() -> void
	{
		if (m_HostContext)
			s_CloseFn(m_HostContext);
	}

	auto ScriptEngine::LoadAssembly(const std::wstring& path) -> void*
	{
		
	}

	auto ScriptEngine::GetManagedFnPointer(const std::wstring& assemblyPath, const std::wstring& typeName, const std::wstring& methodName) -> void*
	{
		load_assembly_and_get_function_pointer_fn loadFn = nullptr;
		s_GetDelegateFn(m_HostContext, hdt_load_assembly_and_get_function_pointer, (void**)&loadFn);

		void* methodPtr = nullptr;
		int result = loadFn(assemblyPath.c_str(), typeName.c_str(), methodName.c_str(), UNMANAGEDCALLERSONLY_METHOD, nullptr, &methodPtr);

		if (result != 0)
		{
			Log::Error("load_assembly_and_get_function_pointer failed: {}", result);
			return nullptr;
		}

		return methodPtr;
	}
}