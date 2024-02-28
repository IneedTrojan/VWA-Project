#pragma once
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "components/FrameBufferResizeComponent.h"
#include "math/BoundingBox.h"
#include "gl/GLObject.h"

namespace scene
{

	inline void GLFWErrorCallback(int error, const char* description) {
		// Handle or log the GLFW error here
		throw std::runtime_error("GLFWErrorCallback");
		std::cerr << "GLFW Error (" << std::codecvt_base::error << "): " << description << std::endl;
	}

	class Window
	{
	protected:

		GLFWwindow* window;
		std::shared_ptr<graphics::FrameBufferObject> fboEntity;

		float aspectRatio;

		math::Rectangle renderViewport;
		math::Rectangle windowSize;
		std::string m_message;

	public:

		GLFWwindow* get_GLFW_window()const
		{
			return window;
		}


		Window() :
			window(nullptr),
			aspectRatio(1),
			renderViewport( 0,0 ,  1000,1000 ),
			windowSize( 0,0 ,  1000,1000 )
		{

		}
		

		void Initialize(entt::registry* registry, int renderWidth, int renderHeight, int windowWidth, int windowHeight, const char* message = "720 fps")
		{
			auto frameBuffer = new graphics::FrameBufferObject();
			auto root = component::Transform2D::CreateRoot(windowWidth, windowHeight);
			frameBuffer->initialize("main", 0, 0, 1, 1, graphics::FrameBufferObjectBase());
			frameBuffer->emplace<component::FBOResizeComponent>(
				frameBuffer, root
			);
			frameBuffer->EmplaceTexture(graphics::FrameBufferTarget::Albedo, graphics::TextureFormat::RGBA8);
			frameBuffer->EmplaceTexture(graphics::FrameBufferTarget::Depth, graphics::TextureFormat::DepthComponent24);
			fboEntity = std::shared_ptr<graphics::FrameBufferObject>(frameBuffer);



			renderViewport = math::Rectangle(0, 0, renderWidth, renderHeight);
			windowSize = math::Rectangle(0, 0, windowWidth, windowHeight);

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_DEPTH_BITS, 24);


			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW");
			}

			aspectRatio = static_cast<float>(renderViewport.y1) / static_cast<float>(renderViewport.y1);

			window = glfwCreateWindow(windowSize.x1, windowSize.y1, m_message.c_str(), nullptr, nullptr);
			if (!window) {
				glfwTerminate();
				throw std::runtime_error("Failed to create GLFW window");
			}

			glfwSetErrorCallback(GLFWErrorCallback);

			glfwMakeContextCurrent(window);

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
				glfwDestroyWindow(window);
				glfwTerminate();
				throw std::runtime_error("Failed to initialize GLAD");
			}
		}
		void Deinitialize()
		{
			if (window != nullptr) {
				glfwDestroyWindow(window);
				window = nullptr; // This is to avoid accidentally referencing a destroyed window
			}

			glfwTerminate();
		}

		void SwapBuffers()const
		{
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		void SetWindowTile(const char* title) const {
			if (window != nullptr && title != nullptr) {
				glfwSetWindowTitle(window, title);
			}
		}

		const std::shared_ptr<component::Transform2D>& GetWindowSize()const
		{
			return fboEntity->get<component::FBOResizeComponent>().transform;
		}

		std::shared_ptr<graphics::FrameBufferObject> GetMainFrameBuffer()
		{
			return fboEntity;
		}

		~Window()
		{
			Deinitialize();
		}

	};
}
