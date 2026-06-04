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
		EP_PROFILE_FN("IndexBuffer::SetData")

		const auto device = DeviceManager::Get()->GetDevice();
		const auto cmd = device->createCommandList();

		if (size > m_Size)
		{
			Log::Warn("Setting data on a buffer that isn't big enough, recreating buffer...");
			m_Size = size;
			CreateBuffer();
		}

		cmd->open();
		cmd->writeBuffer(m_Buffer, data, m_Size, offset);
		cmd->close();

		device->executeCommandList(cmd);
	}

	auto IndexBuffer::CreateBuffer(bool isResize) -> void
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

		auto buffer = device->createBuffer(bufferDesc);
		if (isResize)
		{
			const nvrhi::CommandListHandle cmdList = device->createCommandList();
			cmdList->open();
			cmdList->copyBuffer(buffer, 0, m_Buffer, 0, m_Buffer->getDesc().byteSize);
			cmdList->close();
			device->executeCommandList(cmdList);
		}

		m_Buffer = buffer;
	}

	auto IndexBuffer::CreateCube() -> Ref<IndexBuffer>
	{
		constexpr std::array indices = {
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			8, 9, 10, 8, 10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23
		};

		return CreateRef<IndexBuffer>(indices.data(), indices.size() * sizeof(uint32_t));
	}
}