#include "pch.h"
#include "Platform/Vulkan/Swapchain.h"

#include "Core/Application.h"
#include "Platform/Vulkan/DeviceManagerVK.h"

#include <GLFW/glfw3.h>
#include <nvrhi/vulkan.h>

namespace Eppo
{
	Swapchain::Swapchain(VkSurfaceKHR surface)
		: m_Surface(surface)
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		VkDevice device = dm->GetLogicalDevice()->GetNative();

		// Get swapchain support details
		auto [capabilities, formats, presentModes] = QuerySwapchainSupportDetails();
		m_SurfaceFormat = SelectSurfaceFormat(formats);
		m_PresentMode = SelectPresentMode(presentModes);
		m_Format = m_SurfaceFormat.format;
		m_Extent = SelectExtent(capabilities);

		// Create acquire semaphores
		VkSemaphoreCreateInfo semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		for (uint32_t i = 0; i < g_MaxFramesInFlight; i++)
			VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_AcquireSemaphores[i]), "Failed to create semaphore!");
	}

	Swapchain::~Swapchain()
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		VkDevice device = dm->GetLogicalDevice()->GetNative();

		m_Images.clear();

		for (size_t i = 0; i < m_PresentSemaphores.size(); i++)
			vkDestroySemaphore(device, m_PresentSemaphores.at(i), nullptr);

		for (size_t i = 0; i < m_AcquireSemaphores.size(); i++)
			vkDestroySemaphore(device, m_AcquireSemaphores.at(i), nullptr);

		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
		vkDestroySurfaceKHR(dm->GetVulkanInstance(), m_Surface, nullptr);
	}

	auto Swapchain::BeginFrame() -> bool
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		VkDevice device = dm->GetLogicalDevice()->GetNative();

		const auto& semaphore = m_AcquireSemaphores.at(m_AcquireIndex);

		constexpr uint32_t maxAttempts = 3;
		VkResult result;

		for (uint32_t attempt = 0; attempt < maxAttempts; attempt++) // switch to ++attempt
		{
			result = vkAcquireNextImageKHR(device, m_Swapchain, UINT64_MAX, semaphore, nullptr, &m_SwapchainIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			{
				// Resize swapchain
				CreateSwapchain();
			}
			else
			{
				break;
			}
		}

		m_AcquireIndex = (m_AcquireIndex + 1) % m_AcquireSemaphores.size();

		if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
		{
			nvrhi::vulkan::IDevice* vkNvrhiDevice(dm->GetDevice()->getNativeObject(nvrhi::ObjectTypes::Nvrhi_VK_Device));
			vkNvrhiDevice->queueWaitForSemaphore(nvrhi::CommandQueue::Graphics, semaphore, 0);
			return true;
		}
		
		return false;
	}

	auto Swapchain::Present() -> bool
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		VkDevice device = dm->GetLogicalDevice()->GetNative();
		nvrhi::vulkan::IDevice* vkNvrhiDevice(dm->GetDevice()->getNativeObject(nvrhi::ObjectTypes::Nvrhi_VK_Device));

		const auto& semaphore = m_PresentSemaphores.at(m_SwapchainIndex);

		vkNvrhiDevice->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, semaphore, 0);
		vkNvrhiDevice->executeCommandLists(nullptr, 0);

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &semaphore,
			.swapchainCount = 1,
			.pSwapchains = &m_Swapchain,
			.pImageIndices = &m_SwapchainIndex,
		};

		VkResult result = vkQueuePresentKHR(dm->GetLogicalDevice()->GetPresentQueue(), &presentInfo);
		if (!(result == VK_SUCCESS || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR))
			return false;

		// Explicit sync
		vkQueueWaitIdle(dm->GetLogicalDevice()->GetPresentQueue());

		while (m_FramesInFlight.size() >= g_MaxFramesInFlight)
		{
			auto query = m_FramesInFlight.front();
			m_FramesInFlight.pop();

			vkNvrhiDevice->waitEventQuery(query);
			m_QueryPool.emplace_back(query);
		}

		nvrhi::EventQueryHandle query;
		if (!m_QueryPool.empty())
		{
			query = m_QueryPool.back();
			m_QueryPool.pop_back();
		}
		else
		{
			query = vkNvrhiDevice->createEventQuery();
		}

		vkNvrhiDevice->resetEventQuery(query);
		vkNvrhiDevice->setEventQuery(query, nvrhi::CommandQueue::Graphics);
		m_FramesInFlight.push(query);

		return true;
	}

	auto Swapchain::CreateSwapchain(uint32_t width, uint32_t height) -> void
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		VkDevice device = dm->GetLogicalDevice()->GetNative();

		m_Images.clear();

		auto [capabilities, formats, presentModes] = QuerySwapchainSupportDetails();
		const auto& indices = dm->GetPhysicalDevice()->GetQueueFamilyIndices();
		VkSwapchainKHR oldSwapchain = m_Swapchain ? m_Swapchain : nullptr;

		if (width == 0 || height == 0)
			m_Extent = SelectExtent(capabilities);
		else
			m_Extent = { width, height };

		VkSwapchainCreateInfoKHR swapchainInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface,
			.minImageCount = capabilities.minImageCount,
			.imageFormat = m_Format,
			.imageColorSpace = m_SurfaceFormat.colorSpace,
			.imageExtent = m_Extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageSharingMode = indices.Graphics == indices.Present ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = m_PresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = oldSwapchain,
		};

		VK_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &m_Swapchain), "Failed to create swapchain!");
		EP_ASSERT(m_Swapchain);

		if (oldSwapchain)
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr);

		uint32_t swapchainImageCount = 0;
		VK_CHECK(vkGetSwapchainImagesKHR(device, m_Swapchain, &swapchainImageCount, nullptr), "Failed to get swapchain images!");
		EP_ASSERT(swapchainImageCount >= 2);
		m_PresentSemaphores.resize(swapchainImageCount);

		std::vector<VkImage> images(swapchainImageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(device, m_Swapchain, &swapchainImageCount, images.data()), "Failed to get swapchain images!");

		constexpr VkSemaphoreCreateInfo semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		for (uint32_t i = 0; i < swapchainImageCount; i++)
		{
			// Create image views
			auto& image = m_Images.emplace_back();
			image.NativeImage = images.at(i);

			nvrhi::TextureDesc textureDesc{
				.width = m_Extent.width,
				.height = m_Extent.height,
				.format = nvrhi::Format::RGBA8_UNORM,
				.dimension = nvrhi::TextureDimension::Texture2D,
				.debugName = std::format("Swapchain Image {}", i),
				.isRenderTarget = true,
				.initialState = nvrhi::ResourceStates::Present,
				.keepInitialState = true,
			};

			image.Texture = dm->GetDevice()->createHandleForNativeTexture(nvrhi::ObjectTypes::VK_Image, nvrhi::Object(image.NativeImage), textureDesc);

			nvrhi::FramebufferDesc framebufferDesc{};
			framebufferDesc.addColorAttachment(image.Texture);

			image.Framebuffer = dm->GetDevice()->createFramebuffer(framebufferDesc);

			// Create present semaphores
			VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_PresentSemaphores[i]), "Failed to create semaphore!");
		}
	}

	auto Swapchain::Resize(uint32_t width, uint32_t height) -> void
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		VkDevice device = dm->GetLogicalDevice()->GetNative();

		dm->GetDevice()->waitForIdle();
		CreateSwapchain(width, height);
	}

	auto Swapchain::QuerySwapchainSupportDetails() const -> SwapchainSupportDetails
	{
		const auto& dm = std::static_pointer_cast<DeviceManagerVK>(DeviceManager::Get());
		const auto& physicalDevice = dm->GetPhysicalDevice();

		SwapchainSupportDetails details;

		// Capabilities
		VK_CHECK(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->GetNative(), m_Surface, &details.Capabilities),
			"Failed to get swapchain surface capabilities!"
		);

		// Formats
		uint32_t formatCount = 0;
		VK_CHECK(
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->GetNative(), m_Surface, &formatCount, nullptr),
			"Failed to get swapchain surface format!"
		);

		if (formatCount > 0)
		{
			details.Formats.resize(formatCount);
			VK_CHECK(
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->GetNative(), m_Surface, &formatCount, details.Formats.data()),
				"Failed to get swapchain surface format!"
			);
		}

		// Present modes
		uint32_t presentModeCount = 0;
		VK_CHECK(
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->GetNative(), m_Surface, &presentModeCount, nullptr),
			"Failed to get swapchain present modes!"
		);

		if (presentModeCount > 0)
		{
			details.PresentModes.resize(presentModeCount);
			VK_CHECK(
				vkGetPhysicalDeviceSurfacePresentModesKHR(
					physicalDevice->GetNative(), m_Surface, &presentModeCount, details.PresentModes.data()
				),
				"Failed to get swapchain present modes!"
			);
		}

		return details;
	}

	auto Swapchain::SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) -> VkSurfaceFormatKHR
	{
		VkSurfaceFormatKHR surfaceFormat{};

		for (const auto& format : surfaceFormats)
		{
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surfaceFormat = format;
				break;
			}
		}

		if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
		{
			Log::Warn("Can't find requested swapchain format, falling back to first found!");
			surfaceFormat = surfaceFormats.back();
		}

		return surfaceFormat;
	}

	auto Swapchain::SelectPresentMode(const std::vector<VkPresentModeKHR>& presentModes) -> VkPresentModeKHR
	{
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = mode;
				break;
			}
		}

		return presentMode;
	}

	auto Swapchain::SelectExtent(const VkSurfaceCapabilitiesKHR& capabilities) const -> VkExtent2D
	{
		VkExtent2D extent = capabilities.currentExtent;

		if (capabilities.currentExtent.width == UINT32_MAX)
		{
			int width = 0;
			int height = 0;
			glfwGetFramebufferSize(Application::Get().GetWindow()->GetNative(), &width, &height);

			extent = { .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) };
			extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		return extent;
	}
}