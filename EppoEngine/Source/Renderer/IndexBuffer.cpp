#include "pch.h"
#include "Renderer/IndexBuffer.h"

#include "Renderer/DeviceManager.h"

namespace Eppo
{
	IndexBuffer::IndexBuffer(uint64_t size)
		: m_Size(size)
	{
		CreateBuffer();
	}

	IndexBuffer::IndexBuffer(const void* data, uint64_t size)
		: m_Size(size)
	{
		CreateBuffer();
		SetData(data, m_Size);
	}

	auto IndexBuffer::SetData(const void* data, uint64_t size, uint64_t offset) -> void
	{
		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();
		const nvrhi::CommandListHandle cmd = device->createCommandList();

		CreateBuffer();

		cmd->open();
		cmd->writeBuffer(m_Buffer, data, m_Size);
		cmd->close();

		device->executeCommandList(cmd);
	}

	auto IndexBuffer::CreateBuffer() -> void
	{
		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();

		auto bufferDesc = nvrhi::BufferDesc()
			.setByteSize(m_Size)
			.setDebugName("VertexBuffer")
			.setIsIndexBuffer(true)
			.enableAutomaticStateTracking(nvrhi::ResourceStates::IndexBuffer);

		m_Buffer = device->createBuffer(bufferDesc);
	}
}