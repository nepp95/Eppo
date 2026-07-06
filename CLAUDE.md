# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Eppo is a C++20 game engine (`EppoEngine` static library) with an ImGui-based editor (`EppoEditor` executable). Rendering is Vulkan via the NVRHI abstraction layer; ECS via EnTT; C# scripting via .NET runtime hosting. `EppoRuntime` is a stub and is not currently added to the build.

## Prerequisites / Environment

Required environment variables (configure fails without them):
- `VCPKG_ROOT` — vcpkg checkout (dependencies come from the vcpkg manifest `vcpkg.json`)
- `VULKAN_SDK` — Vulkan SDK install
- `DOTNET_ROOT` — .NET SDK (used to build the managed scripting assembly on Windows)

`CMake/Ports/` contains vcpkg overlay ports for `nvrhi` and `epposcriptcore` (the scripting glue library, sourced from https://gitlab.com/nepp95/epposcriptcore).

## Build Commands

Uses CMake presets (compiler is clang/clang-cl on both platforms):

```
cmake --preset=windows-debug          # configure (linux-debug on Linux)
cmake --build --preset=windows-debug  # build
```

Preset variants: `windows-debug` (Debug), `windows-release` (RelWithDebInfo), `windows-dist` (Release), each with a `linux-` counterpart. The presets define everything (compiler, build type, toolchain, build dir) — no extra flags needed. Build trees live in `build/debug`, `build/release`, `build/dist`; the debug editor executable is `build/debug/EppoEditor/Debug/EppoEditor.exe`, with runtime DLLs, `Resources/`, and the managed scripting assembly copied next to it by post-build steps.

There is a `BUILD_TESTING` option and a `unittest-cpp` vcpkg dependency, but no test target currently exists in the CMake files.

## Architecture

Three CMake targets/directories:

- **EppoEngine/** — static library, all engine code under `Source/`. Its `Source/` dir is a public include directory, so headers are included as e.g. `#include "Renderer/Renderer.h"`. Precompiled header at `Source/pch.h`.
- **EppoEditor/** — the editor executable; `EditorLayer` plus dockable panels in `Source/Panels/` (managed by `PanelManager`). `Resources/` (shaders, fonts, meshes, new-project templates) is copied incrementally to the output dir via a stamp file (`_ResourcesSync`).
- **EppoRuntime/** — placeholder, not built.

### Application framework (EppoEngine/Source/Core/)

`main()` lives in `Core/EntryPoint.h`; a client (the editor) includes it once and implements `Eppo::CreateApplication(argc, argv)` returning an `Application` subclass. `Application` is a singleton (`Application::Get()`) owning the `Window` (GLFW), `DeviceManager`, a layer stack (`PushLayer<T>()`), and the `ImGuiLayer`. Events (`Event/`) are dispatched down the layer stack via `OnEvent`.

Conventions from `Core/Base.h`:
- `Ref<T>` = `std::shared_ptr`, `CreateRef<T>()`; `ScopedPtr<T>` = `std::unique_ptr`, `CreateScopedPtr<T>()`
- `EP_ASSERT(cond, msg)` — logs and debug-breaks in Debug/Release, compiled out in Dist
- Config macros: `EP_DEBUG` / `EP_RELEASE` / `EP_DIST`; platform macros: `EP_PLATFORM_WINDOWS` / `EP_PLATFORM_LINUX`
- Tracy profiling (`EP_PROFILE_FN`, `EP_FRAME_MARK`) is enabled in Debug and Release configs; global `new`/`delete` are overridden for memory tracking
- Codebase uses trailing return type style (`auto Foo() -> void`)

### Renderer (EppoEngine/Source/Renderer/ + Platform/Vulkan/)

Renderer classes (`Shader`, `Pipeline`, `Framebuffer`, buffers, `Image`, `Mesh`) are written against NVRHI handles; the Vulkan-specific device/swapchain/shader-compilation code is isolated in `Platform/Vulkan/`. Shaders are GLSL (`.vert`/`.frag` in `EppoEditor/Resources/Shaders/`), compiled at runtime through DXC with a disk cache (`Resources/Shaders/Cache`) and reflected with spirv-cross. `SceneRenderer` handles scene-level drawing on top of the lower-level `Renderer`.

### Scene & assets

`Scene/` is an EnTT-based ECS (`Scene`, `Entity` wrapper, `Components.h`); scenes and projects are serialized to JSON (nlohmann-json, helpers in `Utility/Json.h`). `Project/` models a user project; `Asset/` provides an `AssetManager`/`AssetImporter` layer with UUID-keyed metadata (glTF meshes via tinygltf).

### Scripting (EppoEngine/Source/Scripting/)

`ScriptEngine` is a static singleton wrapping `EppoScriptCore.Native` (from the vcpkg overlay port), which hosts the .NET runtime. On Windows, the editor build also compiles `EppoScriptCore.Managed.csproj` (C# sources from the port) with `dotnet build` and copies the resulting DLL + `runtimeconfig.json` beside the editor exe. `ScriptEngine::Init` takes the runtime config path, then `LoadUserAssembly` loads user C# assemblies; engine callbacks (logging, input) are registered into the managed side.
