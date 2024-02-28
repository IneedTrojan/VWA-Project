#pragma once
#include "Entity.h"
#include "IComponent.h"
#include "SystemBase.h"
#include "Transform.h"
#include "gl/GLObject.h"
using Color = glm::vec<4, uint8_t>;
namespace component
{
	class DirectionalLight: IComponent<Transform>
	{
		int32_t numCascades = 1;

		Color lightColor = Color(255, 255, 255, 255);
		DirectionalLight()
		{
			
		}
		void initialize(Color color, glm::vec3 forward, int32_t _numCascades)
		{
			lightColor = color;
			GetStoredComponent<Transform>().LookAt(forward);
			numCascades = _numCascades;
		}
		
	};
	

	class Light
	{
		class System : SystemBase
		{
			Entity<graphics::RenderTexture> frameBuffer;




			void Update() override
			{



			}

		};
	};



}
