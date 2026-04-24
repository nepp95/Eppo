#include "pch.h"
#include "Scene/Entity.h"

namespace Eppo
{
	Entity::Entity(EntityHandle entityHandle, Scene* scene)
		: m_EntityHandle(entityHandle), m_Scene(scene)
	{
	}
}