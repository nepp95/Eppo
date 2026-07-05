#pragma once

#include <EppoScriptCore.Native/Assembly.h>
#include <EppoScriptCore.Native/ScriptClass.h>

namespace Eppo
{
    class ScriptEngine
    {
    public:
        ScriptEngine(const ScriptEngine&) = delete;
        ScriptEngine& operator=(const ScriptEngine&) = delete;

        static auto Init(const std::filesystem::path& runtimeConfigPath) -> bool;
        static auto Shutdown() -> void;

        static auto LoadUserAssembly(const std::filesystem::path& path) -> void;
        [[nodiscard]] static auto IsRuntimeLoaded() -> bool;

        [[nodiscard]] static auto GetClasses() -> const std::vector<EppoScriptCore::ScriptClass>&;
        [[nodiscard]] static auto FindClassIndex(const std::string& fullName) -> int32_t;

        ~ScriptEngine();

    private:
        ScriptEngine() = default;

        static auto LogCallback(uint8_t level, const char* message) -> void;
        static auto InputIsKeyDownCallback(uint32_t keyCode) -> bool;
        static auto ErrorCallback(const std::string& message) -> void;

        std::unique_ptr<EppoScriptCore::Assembly> m_CoreAssembly;

        static std::unique_ptr<ScriptEngine> s_Instance;
    };
}
