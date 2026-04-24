#include "pch.h"
#include "Renderer/VertexBuffer.h"

#include "Renderer/DeviceManager.h"

namespace Eppo
{
	VertexBuffer::VertexBuffer(uint64_t size)
		: m_Size(size)
	{
		CreateBuffer();
	}

	VertexBuffer::VertexBuffer(const void* data, uint64_t size)
		: m_Size(size)
	{
		CreateBuffer();
		SetData(data, m_Size);
	}

	auto VertexBuffer::SetData(const void* data, uint64_t size, uint64_t offset) -> void
	{
		EP_ASSERT(UINT64_MAX - size > offset);

		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();
		const nvrhi::CommandListHandle cmdList = device->createCommandList();

		cmdList->open();
		cmdList->writeBuffer(m_Buffer, data, size, offset);
		cmdList->close();

		device->executeCommandList(cmdList);
	}

	auto VertexBuffer::CreateBuffer() -> void
	{
		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();

		auto bufferDesc = nvrhi::BufferDesc()
			.setByteSize(m_Size)
			.setDebugName("VertexBuffer")
			.setIsVertexBuffer(true)
			.enableAutomaticStateTracking(nvrhi::ResourceStates::VertexBuffer);

		m_Buffer = device->createBuffer(bufferDesc);
	}
}