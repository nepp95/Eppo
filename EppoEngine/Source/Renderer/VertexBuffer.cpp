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

		if (size > m_Size)
		{
			m_Size = size;
			CreateBuffer();
		}
		else
			m_Size = size;

		cmdList->open();
		cmdList->writeBuffer(m_Buffer, data, size, offset);
		cmdList->close();

		device->executeCommandList(cmdList);
	}

	auto VertexBuffer::CreateBuffer() -> void
	{
		const nvrhi::DeviceHandle device = DeviceManager::Get()->GetDevice();

		nvrhi::BufferDesc bufferDesc{
			.byteSize = m_Size,
			.debugName = "VertexBuffer",
			.isIndexBuffer = true,
			.initialState = nvrhi::ResourceStates::VertexBuffer,
			.keepInitialState = true,
		};

		m_Buffer = device->createBuffer(bufferDesc);
	}
}