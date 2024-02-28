#pragma once
#include "components/SystemBase.h"
#include "Scene/Input.h"
#include "Window.h"
#include "renderer/IRenderPipeline.h"
#include "renderer/IRenderSystem.h"
#include "components/Entity.h"

namespace scene
{


	class SceneClass {
	public:
		InputSystem inputSystem;
		Window screen;

		entt::registry registry;
		component::ComponentSystem componentSystems;
		renderer::RenderSystem renderSystems;

		renderer::IRenderPipeline* pipeline;

		SceneClass() = default;
		SceneClass(renderer::IRenderPipeline* _pipeline) : pipeline(_pipeline)
		{

		}

		void Initialize()
		{
			screen.Initialize(&registry, 1024 * 2, 1024*2, 1024 * 1.5, 1024*1.5);
			inputSystem.Initialize(&screen);

			componentSystems.ForEach([this](component::SystemBase* sys) {sys->Initialize(this); });
			renderSystems.ForEach([this](renderer::IRenderSystem* sys) {sys->Initialize(this); });
			if(pipeline)
			{
				pipeline->Initialize(this);
			}
		}

		void BindPipeline(renderer::IRenderPipeline* _pipeline)
		{
			pipeline = _pipeline;
		}
		component::Entity CreateEntity()
		{
			return component::Entity(&registry);
		}

	};
}
