#include "pch.h"
#include "Scripting/ScriptEngine.h"

#include "Core/Input.h"

namespace Eppo
{
    std::unique_ptr<ScriptEngine> ScriptEngine::s_Instance = nullptr;

    ScriptEngine::~ScriptEngine()
    {
        m_CoreAssembly.reset();
    }

    auto ScriptEngine::Init(const std::filesystem::path& runtimeConfigPath) -> bool
    {
        if (s_Instance)
            return true;

        s_Instance = std::unique_ptr<ScriptEngine>(new ScriptEngine());

        EppoScriptCore::NativeCallbacks callbacks;
        callbacks.Log = LogCallback;
        callbacks.InputIsKeyDown = InputIsKeyDownCallback;

        s_Instance->m_CoreAssembly = std::make_unique<EppoScriptCore::Assembly>(
            ErrorCallback,
            runtimeConfigPath.wstring(),
            callbacks
        );

        return s_Instance->IsRuntimeLoaded();
    }

    auto ScriptEngine::Shutdown() -> void
    {
        if (!s_Instance)
            return;

        s_Instance->m_CoreAssembly.reset();
        s_Instance.reset();
    }

    auto ScriptEngine::LoadUserAssembly(const std::filesystem::path& path) -> void
    {
        if (!s_Instance || !s_Instance->m_CoreAssembly)
            return;

        s_Instance->m_CoreAssembly->LoadUserAssembly(path.wstring());
    }

    auto ScriptEngine::GetClasses() -> const std::vector<EppoScriptCore::ScriptClass>&
    {
        static const std::vector<EppoScriptCore::ScriptClass> empty;

        if (!s_Instance || !s_Instance->m_CoreAssembly)
            return empty;

        return s_Instance->m_CoreAssembly->GetClasses();
    }

    auto ScriptEngine::FindClassIndex(const std::string& fullName) -> int32_t
    {
        if (!s_Instance || !s_Instance->m_CoreAssembly)
            return -1;

        return s_Instance->m_CoreAssembly->FindClassIndex(fullName);
    }

    auto ScriptEngine::LogCallback(const uint8_t level, const char* message) -> void
    {
        switch (level)
        {
            case 0: Log::Trace("{}", message); break;
            case 1: Log::Info("{}", message); break;
            case 2: Log::Warn("{}", message); break;
            case 3: Log::Error("{}", message); break;
            default: Log::Error("{}", message); break;
        }
    }

    auto ScriptEngine::InputIsKeyDownCallback(const uint32_t keyCode) -> bool
    {
        return Input::IsKeyPressed(static_cast<KeyCode>(keyCode));
    }

    auto ScriptEngine::ErrorCallback(const std::string& message) -> void
    {
        Log::Error("{}", message);
    }

    auto ScriptEngine::IsRuntimeLoaded() -> bool
    {
        return s_Instance && s_Instance->m_CoreAssembly != nullptr;
    }

}
