#pragma once

#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace Eppo
{
	class Input
	{
	public:
		static auto IsKeyPressed(KeyCode key) -> bool;
		static auto IsMouseButtonPressed(MouseCode button) -> bool;
		
		static auto GetMousePosition() -> glm::vec2;
		static auto GetMouseX() -> float;
		static auto GetMouseY() -> float;
	};
}