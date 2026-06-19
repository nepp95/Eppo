#include "pch.h"
#include "Renderer/StorageBuffer.h"

#include "Renderer/DeviceManager.h"

namespace Eppo
{
	StorageBuffer::StorageBuffer(uint32_t structStride, uint64_t initialSize, const std::string& debugName)
		: m_Stride(structStride), m_DebugName(debugName)
	{
		EP_PROFILE_FN("StorageBuffer::StorageBuffer");

		m_Size = initialSize >= m_Stride ? initialSize : m_Stride;
		CreateBuffer();
	}

	auto StorageBuffer::SetData(const void* data, uint64_t size, uint64_t offset) -> void
	{
		EP_PROFILE_FN("StorageBuffer::SetData");

		const auto device = DeviceManager::Get()->GetDevice();
		const auto cmd = device->createCommandList();

		if (size > m_Size)
		{
			Log::Warn("Setting data on a buffer that isn't big enough, recreating buffer...");
			m_Size = size;
			CreateBuffer();
		}

		cmd->open();
		cmd->writeBuffer(m_Buffer, data, m_Size);
		cmd->close();

		device->executeCommandList(cmd);
	}

	auto StorageBuffer::CreateBuffer() -> void
	{
		EP_PROFILE_FN("StorageBuffer::CreateBuffer");

		const auto device = DeviceManager::Get()->GetDevice();

		nvrhi::BufferDesc bufferDesc{
			.byteSize = m_Size,
			.structStride = m_Stride,
			.debugName = m_DebugName,
			.initialState = nvrhi::ResourceStates::ShaderResource,
			.keepInitialState = true,
		};

		m_Buffer = device->createBuffer(bufferDesc);
	}
}