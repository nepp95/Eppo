#include "pch.h"
#include "ImGui/ImGuiLayer.h"

#include "Core/Application.h"
#include "Renderer/DeviceManager.h"

// TODO: TEMPORARY
#include "Platform/Vulkan/DeviceManagerVK.h"
#include "Platform/Vulkan/Swapchain.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>

namespace Eppo
{
	struct ImGuiViewportData
	{
		bool WindowOwned = false;
		ScopedPtr<Swapchain> Swapchain = nullptr;
		ScopedPtr<ImGuiRenderer> Renderer = nullptr;
	};

	auto ImGuiLayer::OnAttach() -> void
	{
		// Create imgui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup imgui config
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Load font
		constexpr float fontSize = 14.0f;
		const auto fontPath = FS::GetResourcesDirectory() / "Fonts" / "Roboto-Regular.ttf";
		io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSize);

		// Setup style
		ImGui::StyleColorsDark();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Initialize imgui for glfw
		const auto& dm = DeviceManager::Get();
		const auto& window = Application::Get().GetWindow();

		if (dm->GetParams().API == RendererAPI::Vulkan)
			ImGui_ImplGlfw_InitForVulkan(window->GetNative(), true);
		else
			ImGui_ImplGlfw_InitForOther(window->GetNative(), true);

		// Create renderer
		m_ImGuiRenderer = CreateScopedPtr<ImGuiRenderer>();
		InitPlatformInterface();
	}

	auto ImGuiLayer::OnDetach() -> void
	{
		ImGui::DestroyPlatformWindows();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	auto ImGuiLayer::OnEvent(Event& e) -> void
	{
		if (m_BlockEvents)
		{
			EventType type = e.GetEventType();
			const ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}

		if (e.GetEventType() == EventType::WindowResize)
			m_ImGuiRenderer->Resize();
	}

	auto ImGuiLayer::PrepareRender() -> void
	{
		m_ImGuiRenderer->UpdateFontTexture();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	auto ImGuiLayer::Render() -> void
	{
		const auto& dm = static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());

		ImGui::Render();
		m_ImGuiRenderer->RenderToSwapchain(ImGui::GetMainViewport(), dm->GetSwapchain());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	auto ImGuiLayer::BlockEvents(bool blockEvents) -> void
	{
		m_BlockEvents = blockEvents;
	}

	auto ImGuiLayer::InitPlatformInterface() -> void
	{
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			EP_ASSERT(platformIO.Platform_CreateVkSurface != nullptr);

		platformIO.Renderer_CreateWindow = ImGuiRenderer_CreateWindow;
		platformIO.Renderer_DestroyWindow = ImGuiRenderer_DestroyWindow;
		platformIO.Renderer_SetWindowSize = ImGuiRenderer_SetWindowSize;
		platformIO.Renderer_RenderWindow = ImGuiRenderer_RenderWindow;
		platformIO.Renderer_SwapBuffers = ImGuiRenderer_SwapBuffers;
	}

	auto ImGuiLayer::ImGuiRenderer_CreateWindow(ImGuiViewport* viewport) -> void
	{
		const auto& dm = static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();

		ImGuiViewportData* data = IM_NEW(ImGuiViewportData)();
		viewport->RendererUserData = data;

		VkSurfaceKHR surface = nullptr;
		VkInstance instance = dm->GetVulkanInstance();
		VK_CHECK(platformIO.Platform_CreateVkSurface(viewport, reinterpret_cast<ImU64>(instance), nullptr, reinterpret_cast<ImU64*>(&surface)), "Failed to create vk surface for ImGui!");
		
		data->Swapchain = CreateScopedPtr<Swapchain>(surface);
		data->Swapchain->CreateSwapchain(static_cast<uint32_t>(viewport->Size.x), static_cast<uint32_t>(viewport->Size.y));
		data->Renderer = CreateScopedPtr<ImGuiRenderer>();
		data->WindowOwned = true;
	}

	auto ImGuiLayer::ImGuiRenderer_DestroyWindow(ImGuiViewport* viewport) -> void
	{
		ImGuiViewportData* vd = static_cast<ImGuiViewportData*>(viewport->RendererUserData);
		IM_DELETE(vd);
		viewport->RendererUserData = nullptr;
	}

	auto ImGuiLayer::ImGuiRenderer_SetWindowSize(ImGuiViewport* viewport, ImVec2 size) -> void
	{
		ImGuiViewportData* vd = static_cast<ImGuiViewportData*>(viewport->RendererUserData);
		vd->Swapchain->Resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
	}

	auto ImGuiLayer::ImGuiRenderer_RenderWindow(ImGuiViewport* viewport, void*) -> void
	{
		ImGuiViewportData* vd = static_cast<ImGuiViewportData*>(viewport->RendererUserData);
		vd->Swapchain->BeginFrame();
		vd->Renderer->UpdateFontTexture();
		vd->Renderer->RenderToSwapchain(viewport, vd->Swapchain);
	}

	auto ImGuiLayer::ImGuiRenderer_SwapBuffers(ImGuiViewport* viewport, void*) -> void
	{
		ImGuiViewportData* vd = static_cast<ImGuiViewportData*>(viewport->RendererUserData);
		vd->Swapchain->Present();
	}
}