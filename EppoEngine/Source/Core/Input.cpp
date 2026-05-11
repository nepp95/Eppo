#include "pch.h"
#include "Core/Input.h"

#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace Eppo
{
	auto Input::IsKeyPressed(KeyCode key) -> bool
	{
		GLFWwindow* window = Application::Get().GetWindow()->GetNative();
		const int keyState = glfwGetKey(window, key);

		return keyState == GLFW_PRESS;
	}

	auto Input::IsMouseButtonPressed(MouseCode button) -> bool
	{
		GLFWwindow* window = Application::Get().GetWindow()->GetNative();
		const int buttonState = glfwGetMouseButton(window, button);

		return buttonState == GLFW_PRESS;
	}

	auto Input::GetMousePosition() -> glm::vec2
	{
		GLFWwindow* window = Application::Get().GetWindow()->GetNative();

		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { static_cast<float>(xPos), static_cast<float>(yPos) };
	}

	auto Input::GetMouseX() -> float
	{
		GLFWwindow* window = Application::Get().GetWindow()->GetNative();

		double pos;
		glfwGetCursorPos(window, &pos, nullptr);

		return static_cast<float>(pos);
	}

	auto Input::GetMouseY() -> float
	{
		GLFWwindow* window = Application::Get().GetWindow()->GetNative();

		double pos;
		glfwGetCursorPos(window, nullptr, &pos);

		return static_cast<float>(pos);
	}
}