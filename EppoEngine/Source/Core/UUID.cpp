#include "pch.h"
#include "Core/UUID.h"

namespace Eppo
{
	UUID::UUID()
	{
		m_UUID = Utils::GenerateRandomUInt64();
	}

	UUID::UUID(uint64_t id)
		: m_UUID(id)
	{}
}