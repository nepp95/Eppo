#include "pch.h"
#include "Core/UUID.h"

namespace Eppo
{
	UUID::UUID()
	{
		m_UUID = Utils::GenerateRandomUInt64();
	}
}