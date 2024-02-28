#pragma once
#include <glm/vec3.hpp>

#include "IComponent.h"
#include "SystemBase.h"
#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <Scene/Input.h>

#include "Transform.h"
#include "Scene/Scene.h"
#include "utility/ToString.h"
#include <glm/gtc/type_ptr.hpp>
#include "components/Entity.h"


namespace component
{

	class DirectionalLightController
	{
	public:

		DirectionalLightController() :
			mask(1u)
		{

		}
		void initialize(float sens, float radius)
		{
			Sens = sens;
			Radius = radius;
		}

		void setMask(int32_t layer)
		{
			mask = 1u << layer;
		}

		float Sens = 0.01f;
		float Radius = 1.0f;
		GLbitfield mask;



		class System : public SystemBase
		{
		public:
			System() : SystemBase() {}
		private:

			scene::SceneClass* m_scene = nullptr;
			GLbitfield ActiveCamera = 0;


		public:
			void Initialize(scene::SceneClass* scene) override
			{
				m_scene = scene;

			}
			void UpdateMask()
			{
				std::bitset<10> bits;
				for (int i = 0; i < 10; i++)
				{
					bits[i] = m_scene->inputSystem.IsHitBegin(GLFW_KEY_0 + i);
				}
				if (bits.to_ullong())
				{
					ActiveCamera ^= bits.to_ullong();
				}
			}



			void Update() override
			{
				UpdateMask();

				const auto view = m_scene->registry.view<DirectionalLightController>();

				for (const auto id : view)
				{
					EntityView entity(&m_scene->registry, id);

					const auto& controller = entity.get<DirectionalLightController>();
					if (controller.mask & ActiveCamera)
					{
						Transform& transform = entity.get<Transform>();

						glm::vec2 mouseDelta;
						m_scene->inputSystem.getMouseDelta(&mouseDelta.x, &mouseDelta.y);
						mouseDelta = -mouseDelta;

						if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0)
						{
							glm::quat rotation(glm::vec3(mouseDelta.y, mouseDelta.x, 0) * controller.Sens);
							transform.rotation() = glm::normalize(transform.rotation() * glm::quat(rotation));
						}
						transform.position() = transform.RotatePoint(glm::vec3(0, 0, controller.Radius));


					}

				}
			}
		};




	};

	
	class CameraController
	{
	public:

		CameraController() :
			Velocity(0), Sens(0), Speed(0), Drag(0), mask(1u)
		{

		}
		void initialize(float sens, float speed, float drag)
		{
			Sens = sens;
			Speed = speed;
			Drag = drag;
		}

		void setMask(int32_t layer)
		{
			mask = 1u << layer;
		}

		glm::vec3 Velocity;
		float Sens;
		float Speed;
		float Drag;
		GLbitfield mask;
		bool IsAxisAligned;

	};

	class CameraControllerSystem : public SystemBase
	{
	public:
		CameraControllerSystem(){}
	private:
		scene::SceneClass* m_scene = nullptr;
		GLbitfield ActiveCamera = 0;


	public:
		void Initialize(scene::SceneClass* scene) override
		{
			m_scene = scene;

		}
		void UpdateMask()
		{
			std::bitset<10> bits;
			for (int i = 0; i < 10; i++)
			{
				bits[i] = m_scene->inputSystem.IsHitBegin(GLFW_KEY_0 + i);
				if(bits[i])
				{
					std::cout << "ToggleCamera__" << i << "__"<< !(ActiveCamera&1ULL<<i)<<"\n";
				}
			}
			if (bits.to_ullong())
			{
				ActiveCamera ^= bits.to_ullong();
			}
		}



		void Update() override
		{
			UpdateMask();

			auto view = m_scene->registry.view<CameraController>();

			for (const auto id : view)
			{
				EntityView entity(&m_scene->registry, id);
				auto& cameraController = entity.get<CameraController>();
				if(cameraController.mask&ActiveCamera)
				{
					Transform& transform = entity.get<Transform>();

					glm::vec2 mouseDelta;
					m_scene->inputSystem.getMouseDelta(&mouseDelta.x, &mouseDelta.y);
					mouseDelta = -mouseDelta;



					float horizontal, vertical;
					m_scene->inputSystem.getMovementInput(&horizontal, &vertical);
					float vertical_z = static_cast<float>(m_scene->inputSystem.IsPressed(GLFW_KEY_R)- m_scene->inputSystem.IsPressed(GLFW_KEY_F));
					

					cameraController.Speed += static_cast<float>(m_scene->inputSystem.IsPressed(GLFW_KEY_K) - m_scene->inputSystem.IsPressed(GLFW_KEY_L))*(cameraController.Speed*0.01+0.001);

					cameraController.Velocity += transform.RotatePoint(glm::vec3(vertical, vertical_z, horizontal) * cameraController.Speed);
					cameraController.Velocity *= cameraController.Drag;

					if (glm::abs(cameraController.Velocity.x) > 0.01f ||
						glm::abs(cameraController.Velocity.y) > 0.01f	||
						glm::abs(cameraController.Velocity.z) > 0.01f)
					{
						transform.position() += cameraController.Velocity*0.1f;
					}
					if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0)
					{
						glm::quat rotation(glm::vec3(mouseDelta.y, mouseDelta.x, 0)*cameraController.Sens);
						transform.rotation() = glm::normalize(transform.rotation() * glm::quat(rotation));
					}
				}
				
			}
		}
	};
}
