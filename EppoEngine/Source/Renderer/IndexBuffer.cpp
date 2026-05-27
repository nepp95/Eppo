#include "pch.h"
#include "Renderer/IndexBuffer.h"

#include "Renderer/DeviceManager.h"

namespace Eppo
{
	IndexBuffer::IndexBuffer(uint64_t size)
		: m_Size(size), m_CpuWritable(true)
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

	auto IndexBuffer::CreateBuffer() -> void
	{
		const auto device = DeviceManager::Get()->GetDevice();

		nvrhi::BufferDesc bufferDesc{
			.byteSize = m_Size,
			.debugName = "IndexBuffer",
			.isIndexBuffer = true,
			.initialState = nvrhi::ResourceStates::IndexBuffer,
			.keepInitialState = true,
		};

		if (m_CpuWritable)
			bufferDesc.cpuAccess = nvrhi::CpuAccessMode::Write;

		m_Buffer = device->createBuffer(bufferDesc);
	}

	auto IndexBuffer::CreateCube() -> Ref<IndexBuffer>
	{
		std::array indices = {
			0, 2, 1, 0, 3, 2,
			4, 6, 5, 4, 7, 6,
			8, 10, 9, 8, 11, 10,
			12, 14, 13, 12, 15, 14,
			16, 18, 17, 16, 19, 18,
			20, 22, 21, 20, 23, 22
		};

		return CreateRef<IndexBuffer>(indices.data(), indices.size() * sizeof(uint32_t));
	}
}