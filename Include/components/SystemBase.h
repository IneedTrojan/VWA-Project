#pragma once
#include <entt/entt.hpp>
#include <type_traits>
#include <typeindex>
namespace scene
{
	class SceneClass;
}

namespace component
{
	class SystemBase
	{
	public:

		SystemBase() {}
		virtual void Update() = 0;
		virtual void Initialize(scene::SceneClass* scene) = 0;

		virtual ~SystemBase() {}

	};


	class ComponentSystem
	{
		std::vector<std::weak_ptr<SystemBase>> systems;
		std::vector<std::type_index> types;
	public:
		template<typename T>
		std::shared_ptr<T> emplace()requires std::is_base_of_v<SystemBase, T>
		{
			std::shared_ptr<T> sys = std::make_shared<T>();
			auto type = std::type_index(typeid(*sys.get()));
			const auto it = std::ranges::find(types.begin(), types.end(), type);
			if (it == types.end())
			{
				systems.emplace_back(std::static_pointer_cast<SystemBase>(sys));
			}
			return sys;
		}
		bool erase(std::type_index type)
		{
			const auto it = std::ranges::find(types.begin(), types.end(), type);
			const auto distance = std::distance(types.begin(), it);
			const bool found = distance < types.size();
			if (found)
			{
				systems.erase(systems.begin() + distance);
				types.erase(types.begin() + distance);
			}
			return found;
		}

		void ForEach(const std::function<void(SystemBase* sys)>& invoke)
		{
			std::vector<SystemBase*> pointers;
			for (size_t i = 0; i < systems.size(); i++)
			{
				if (auto shared = systems[i].lock())
				{
					invoke(shared.get());
				}
				else
				{
					if (erase(types[i]))
					{
						i--;
					}
				}
			}
		}
		template<typename T>
		std::shared_ptr<T> get_shared()requires std::is_base_of_v<SystemBase, T>
		{
			const auto it = std::ranges::find(types.begin(), types.end(), std::type_index(typeid(T)));
			const auto basePtr = systems[std::distance(types.begin(), it)].lock();
			return std::dynamic_pointer_cast<T>(basePtr);
		}
	};


}
