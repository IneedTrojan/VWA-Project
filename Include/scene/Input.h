#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "Window.h"

namespace scene
{

	class InputSystem
	{
		constexpr static int32_t letterBegin = GLFW_KEY_SPACE;
		constexpr static int32_t letterEnd = GLFW_KEY_WORLD_2+1;
		constexpr static int32_t specialBegin = GLFW_KEY_ESCAPE;
		constexpr static int32_t specialEnd = GLFW_KEY_MENU + 1;

		std::array<int32_t, letterEnd+specialEnd-specialBegin-letterBegin> keyData{ 0 };
		scene::Window* window = nullptr;

		double mouseX = 0.0;
		double mouseY = 0.0;

		double deltaX = 0.0;
		double deltaY = 0.0;
		double mouseScroll = 0.0;

	public:
		void PollEvents()
		{
			const auto _win = window->get_GLFW_window();
			glfwPollEvents();

			int32_t index = 0;
			for (auto& counter : keyData)
			{
				int32_t transformed = indexToKey(index++);
				const GLenum keyState = glfwGetKey(_win, transformed);
				if (keyState == GLFW_RELEASE)
				{
					counter = glm::min(0, counter - 1);
				}
				else//keyState == GLFW_PRESS
				{
					counter = glm::max(1, counter + 1);
				}
			}

			double x, y;
			glfwGetCursorPos(_win, &x, &y);
			deltaX = x - mouseX;
			deltaY = y - mouseY;
			mouseX = x;
			mouseY = y;
		}
		int32_t GetKeyState(int32_t key)const
		{
			const int32_t mapped = keyToIndex(key);
			return keyData[mapped];
		}
		int32_t IsPressed(int32_t key)const
		{
			return GetKeyState(key) >0;
		}
		int32_t IsHitBegin(int32_t key)const
		{
			return GetKeyState(key) ==1;
		}
		int32_t IsHitEnd(int32_t key)const
		{
			return GetKeyState(key) == 0;
		}
		void getMouseDelta(float_t* x, float_t* y, float_t* scroll = nullptr)const
		{
			if (x)
			{
				*x = static_cast<float_t>(deltaX);
			}
			if (y)
			{
				*y = static_cast<float_t>(deltaY);
			}
			if (scroll)
			{
				*scroll = static_cast<float_t>(mouseY);
			}
		}
		void getMousePos(float_t* x, float_t* y)const
		{
			if (x)
			{
				*x = static_cast<float_t>(mouseX);
			}
			if (y)
			{
				*y = static_cast<float_t>(mouseY);
			}
		}

		void getMovementInput(float* vertical, float* horizontal)const
		{
			*horizontal = static_cast<float>(IsPressed(GLFW_KEY_D) - IsPressed(GLFW_KEY_A));
			*vertical = static_cast<float>(IsPressed(GLFW_KEY_S) - IsPressed(GLFW_KEY_W));
		}
		void getArrowInput(float* vertical, float* horizontal)const
		{
			*horizontal = static_cast<float>(IsPressed(GLFW_KEY_UP) - IsPressed(GLFW_KEY_DOWN));
			*vertical = static_cast<float>(IsPressed(GLFW_KEY_RIGHT) - IsPressed(GLFW_KEY_LEFT));
		}
		void Initialize(scene::Window* _window)
		{
			window = _window;
		}


	private:

		static int32_t keyToIndex(int32_t key)
		{
			
			return  (key > GLFW_KEY_WORLD_2 ? key - (GLFW_KEY_ESCAPE - GLFW_KEY_WORLD_2) : key) - GLFW_KEY_SPACE;
		}
		static int32_t indexToKey(int32_t index)
		{
			index += GLFW_KEY_SPACE;

			if (index > GLFW_KEY_WORLD_2)
			{
				index -= (GLFW_KEY_ESCAPE - GLFW_KEY_WORLD_2);
			}
			return index;
		}


	};
}
