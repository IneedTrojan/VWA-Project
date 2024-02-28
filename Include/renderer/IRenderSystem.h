#pragma once
#include <typeindex>

#include "math/CullFrustum.h"
#include "math/Layermask.h"
#include "utility/Span.h"
#include "opengl/BufferObject.h"

namespace component
{
	class CameraComponent;
}

namespace scene
{
	class SceneClass;
}


namespace renderer
{
	
	class IRenderSystem
	{
	public:
		virtual void Initialize(scene::SceneClass* scene) = 0;

		virtual void CullCamera(component::EntityView in) = 0;
		virtual void CullForLight(component::EntityView in) = 0;
		
		virtual void RenderColor(component::EntityView in) = 0;

		virtual void RenderGeometry(component::EntityView in) = 0;

		virtual ~IRenderSystem() = default;
	};

	class RenderSystem
	{
		std::vector<std::weak_ptr<IRenderSystem>> systems;
		std::vector<std::type_index> types;
	public:
		template<typename T>
		std::shared_ptr<T> emplace()requires std::is_base_of_v<IRenderSystem, T>
		{
			std::shared_ptr<T> sys = std::make_shared<T>();
			auto type = std::type_index(typeid(*sys.get()));
			const auto it = std::ranges::find(types.begin(), types.end(), type);
			if(it == types.end())
			{
				systems.emplace_back(std::static_pointer_cast<IRenderSystem>(sys));
			}
			return sys;
		}
		bool erase(std::type_index type)
		{
			const auto it = std::ranges::find(types.begin(), types.end(), type);
			const auto distance = std::distance(types.begin(), it);
			const bool found = distance < types.size();
			if(found)
			{
				systems.erase(systems.begin() + distance);
				types.erase(types.begin() + distance);
			}
			return found;
		}
		
		void ForEach(const std::function<void(IRenderSystem* sys)>& invoke)
		{
			std::vector<IRenderSystem*> pointers;
			for(size_t i = 0;i<systems.size();i++)
			{
				if (auto shared = systems[i].lock())
				{
					invoke(shared.get());
				}
				else
				{
					if(erase(types[i]))
					{
						i--;
					}
				}
			}
		}
		template<typename T>
		std::shared_ptr<T> get_shared()requires std::is_base_of_v<IRenderSystem, T>
		{
			const auto it = std::ranges::find(types.begin(), types.end(), std::type_index(typeid(T)));
			const auto ptr = systems[std::distance(types.begin(), it)].lock();
			return std::dynamic_pointer_cast<T>(ptr);
		}
	};




}
