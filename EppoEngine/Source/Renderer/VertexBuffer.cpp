#include "pch.h"
#include "Renderer/VertexBuffer.h"

#include "Renderer/DeviceManager.h"
#include "Renderer/Mesh.h"
#include "Renderer/Vertex.h"

namespace Eppo
{
	VertexBuffer::VertexBuffer(uint64_t size)
		: m_Size(size), m_CpuWritable(true)
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
		EP_PROFILE_FN("VertexBuffer::SetData")

		EP_ASSERT(UINT64_MAX - size > offset);

		const auto device = DeviceManager::Get()->GetDevice();
		const auto cmdList = device->createCommandList();

		if (size > m_Size)
		{
			Log::Warn("Setting data on a buffer that isn't big enough, recreating buffer...");
			m_Size = size;
			CreateBuffer();
		}

		cmdList->open();
		cmdList->writeBuffer(m_Buffer, data, size, offset);
		cmdList->close();

		device->executeCommandList(cmdList);
	}

	auto VertexBuffer::CreateBuffer(bool isResize) -> void
	{
		const auto device = DeviceManager::Get()->GetDevice();

		nvrhi::BufferDesc bufferDesc{
			.byteSize = m_Size,
			.debugName = "VertexBuffer",
			.isVertexBuffer = true,
			.initialState = nvrhi::ResourceStates::VertexBuffer,
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

	auto VertexBuffer::CreateMeshPrimitive(const MeshPrimitiveType type) -> Ref<VertexBuffer>
	{
		switch (type)
		{
			case MeshPrimitiveType::Cube:
			{
				constexpr std::array vertices = {
					Vertex{{ -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }},
					Vertex{{  1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }},
					Vertex{{  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }},
					Vertex{{ -1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }},
					Vertex{{  1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }},
					Vertex{{ -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }},
					Vertex{{ -1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }},
					Vertex{{  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }},
					Vertex{{ -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }},
					Vertex{{ -1.0f, -1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }},
					Vertex{{ -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }},
					Vertex{{ -1.0f,  1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }},
					Vertex{{  1.0f, -1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }},
					Vertex{{  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }},
					Vertex{{  1.0f,  1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }},
					Vertex{{  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }},
					Vertex{{ -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }},
					Vertex{{  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }},
					Vertex{{  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }},
					Vertex{{ -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }},
					Vertex{{ -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }},
					Vertex{{  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }},
					Vertex{{  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }},
					Vertex{{ -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }},
				};

				return CreateRef<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
			}
		}
	}
}