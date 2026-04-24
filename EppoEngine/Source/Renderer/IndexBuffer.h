#pragma once

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	class IndexBuffer
	{
	public:
		IndexBuffer(uint64_t size);
		IndexBuffer(const void* data, uint64_t size);

		auto SetData(const void* data, uint64_t size, uint64_t offset = 0) -> void;

		[[nodiscard]] auto GetBuffer() const -> nvrhi::BufferHandle { return m_Buffer; }
		[[nodiscard]] constexpr auto GetIndexCount() const -> uint64_t { return m_Size / sizeof(uint32_t); }
		[[nodiscard]] constexpr auto GetSize() const -> uint64_t { return m_Size; };

	private:
		auto CreateBuffer() -> void;

	private:
		nvrhi::BufferHandle m_Buffer = nullptr;
		uint64_t m_Size = 0;
	};
}