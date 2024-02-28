#pragma once
#include <tuple>
#include <utility>
#include <type_traits>
#include <iostream>

#include "entt/entt.hpp"

template<typename T, typename... Args>
concept HasInitialize = requires(T t, Args&&... args) {
    { t.initialize(std::forward<Args>(args)...) };
};

template<typename T, typename...Args>
concept hasType = (... || std::is_same_v<T, Args>);




template<typename...ArgsA, typename...ArgsB, typename Function>
void iterate_over_shared_types(std::tuple<ArgsA...>& a, const std::tuple<ArgsB...>& b, Function func)
{
    auto tryFunc = [b, func]<typename T>(T & item) {
        if constexpr (hasType<T, ArgsB...>)
        {
            func(item, std::get<T>(b));
        }
    };
    (std::apply([&](auto&... args) { (tryFunc(args), ...); }, a));
}

namespace component
{
   
    class EntityView {
    protected:
        entt::registry* registry;
        entt::entity entity;
    public:

        EntityView(): EntityView(nullptr, entt::null){}
        EntityView(entt::registry* reg, entt::entity ent) : registry(reg), entity(ent) {}
        ~EntityView() = default;

        template<typename T>
        T& get() const {
            return registry->get<T>(entity);
        }

        template<typename T, typename...Args>
			requires std::is_constructible_v<T, Args...>
        T& emplace(Args&&... args) const {
            return registry->emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T, typename...Args>
            requires std::is_constructible_v<T, Args...>
        T& get_or_emplace(Args&&... args) const {
            return registry->get_or_emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T>
        T* try_get() const {
            return registry->try_get<T>(entity);
        }

        template<typename T, typename...Args>
            requires std::is_constructible_v<T, Args...>
        void replace(Args&&... args) const {
            registry->replace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T, typename...Args>
            requires std::is_constructible_v<T, Args...>
        void replace_or_emplace(Args&&... args) const {
            registry->emplace_or_replace<T>(entity, std::forward<Args>(args)...);
        }
        template<typename T>
        bool has() const {
            return try_get<T>();
        }
        bool valid() const {
            return entity != entt::null && registry->valid(entity);
        }
        template<typename T, typename...Args>
            requires std::is_constructible_v<T>
        T& initialize(Args&&... args)
        {
            T& component = get_or_emplace<T>();
            component.initialize(std::forward<Args...>(args...));
            return component;
        }
        void destroy() {
            if(entity != entt::null)
            {
                registry->destroy(entity);
                entity = entt::null;
            }
            
        }
        EntityView withEntity(entt::entity newEntity)const
        {
            return EntityView(registry, newEntity);
        }
        bool operator == (const EntityView& other)const
        {
            return registry == other.registry && entity == other.entity;
        }
    };
    class Entity : public EntityView {
    public:
        Entity(entt::registry* _registry) : EntityView(_registry, _registry->create()) {}
        Entity() : EntityView(nullptr, entt::null){}

        Entity(Entity&& other) noexcept : EntityView(other.registry, other.entity) {
            other.entity = entt::null; // Set the moved-from entity to null
        }

        Entity& operator=(Entity&& other) noexcept {
            if (this != &other) {
                destroy();

                registry = other.registry;
                entity = other.entity;

                other.entity = entt::null;
            }
            return *this;
        }

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;
        EntityView view()
        {
            return *this;
        }
        ~Entity()
        {
            destroy();
        }
    };
    struct constexprString {
        std::array<char, 256> character;
        constexpr constexprString(const char* str) {
            size_t i = 0;
            for (; str[i] != '\0' && i < character.size(); ++i) {
                character[i] = str[i];
            }
            for (; i < character.size(); ++i) {
                character[i] = '\0';
            }
        }
       
    };

    template<typename Type, constexprString str>
    struct named_component {
        Type get;

    	template<typename...Args>
        requires std::is_constructible_v<Type, Args...>
        named_component(Args&&...args)
        {
            get = Type(std::forward<Args>(args)...);
        }
        operator Type& (){
            return get;
        }
        
    };
   
   /* template<typename A, typename B>
    using typed_component = named_component<A, typeid(B).name()>;*/






    
}












