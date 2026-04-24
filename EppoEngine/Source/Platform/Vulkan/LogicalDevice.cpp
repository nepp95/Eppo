#include "pch.h"
#include "Platform/Vulkan/LogicalDevice.h"

namespace Eppo
{
	LogicalDevice::LogicalDevice(const ScopedPtr<PhysicalDevice>& physicalDevice)
	{
		// Check extension support
		for (const auto& extension : g_DeviceExtensions)
		{
			if (!physicalDevice->IsExtensionSupported(extension))
			{
				Log::Error("Device extension '{}' is not supported by the device!", extension);
				return;
			}
		}

		// Create device queue infos
		const auto& indices = physicalDevice->GetQueueFamilyIndices();
		constexpr float queuePriority = 1.0f;

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		for (const auto& index : indices.GetUniqueIndices())
		{
			auto& queueInfo = queueInfos.emplace_back();
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueCount = 1;
			queueInfo.queueFamilyIndex = index;
			queueInfo.pQueuePriorities = &queuePriority;
		}

		// Create device
		const auto& deviceFeatures = physicalDevice->GetDeviceFeatures();

		VkPhysicalDeviceSynchronization2Features synchronizationFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
			.synchronization2 = VK_TRUE,
		};

		VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
			.pNext = &synchronizationFeatures,
			.timelineSemaphore = VK_TRUE,
		};

		VkDeviceCreateInfo deviceInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &timelineSemaphoreFeatures,
			.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
			.pQueueCreateInfos = queueInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(g_DeviceExtensions.size()),
			.ppEnabledExtensionNames = g_DeviceExtensions.data(),
			.pEnabledFeatures = &deviceFeatures,
		};

		if (g_EnableValidationLayers)
		{
			deviceInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
			deviceInfo.ppEnabledLayerNames = g_ValidationLayers.data();
		}
		else
		{
			deviceInfo.enabledLayerCount = 0;
			deviceInfo.ppEnabledLayerNames = nullptr;
		}

		VK_CHECK(vkCreateDevice(physicalDevice->GetNative(), &deviceInfo, nullptr, &m_Device), "Failed to create device!");
		EP_ASSERT(m_Device);

		// Get device queues
		vkGetDeviceQueue(m_Device, indices.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.Compute, 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_Device, indices.Transfer, 0, &m_TransferQueue);
		vkGetDeviceQueue(m_Device, indices.Present, 0, &m_PresentQueue);
	}

	LogicalDevice::~LogicalDevice()
	{
		vkDestroyDevice(m_Device, nullptr);
	}
}