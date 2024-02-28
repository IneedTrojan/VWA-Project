#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "IDisposable.h"

namespace event {
	template<typename...Args>
	class Event {
	public:
		using Action = std::function<void(Args...)>;
		struct Handle;


	private:


		class Internal {
			friend struct Event<Args...>::Handle;

		public:


			mutable int32_t count = 0;
			mutable std::vector<Action> listeners;
			mutable std::vector<int32_t> HandleKeys;

			void emit(Args... args) {
				for (const auto& listener : listeners) {
					listener(std::forward<Args>(args)...);
				}
			}


			int32_t subscribe(Action func)const {
				int32_t key = count++;
				HandleKeys.emplace_back(key);
				listeners.emplace_back(std::move(func));
				return key;
			}
			template <typename T>
			static void memswap(T& a, T& b) {
				char buffer[sizeof(T)];
				std::memcpy(buffer, &a, sizeof(T));
				std::memcpy(&a, &b, sizeof(T));
				std::memcpy(&b, buffer, sizeof(T));
			}

			void remove(int32_t key)const {
				auto it = std::ranges::find(HandleKeys.begin(), HandleKeys.end(), key);
				size_t distance = std::distance(HandleKeys.begin(), it);

				memswap(listeners.data()[distance], listeners.back());
				memswap(HandleKeys.data()[distance], HandleKeys.back());

				listeners.pop_back();
				HandleKeys.pop_back();
			}
		};
		std::shared_ptr<Internal> self;
	public:
		class Handle {
			std::weak_ptr<Internal> event;
			int32_t key;
		public:

			Handle(const std::shared_ptr<Internal>& _event, int32_t _key) : event(_event), key(_key) {

			}
			Handle(): key(-1) {

			}
			
			void dispose() {
				if (key != -1) {
					if (auto ptr = event.lock()) {
						ptr->remove(key);
						key = -1;
					}
				}

			}
			Handle& operator =(const Handle& other) noexcept = delete;
			Handle(const Handle& other) noexcept = delete;

			Handle& operator =(Handle&& other) noexcept{
				if (&other != this) {
					dispose();
					key = other.key;
					event = std::move(other.event);
					other.key = -1;
				}
				return *this;
			}
			Handle(Handle&& other)noexcept
				:key(other.key), event(std::move(other.event))  {
				
				other.key = -1;
			}

			~Handle() {
				dispose();
			}
		};
		void Reset() {
			self = std::make_shared<Internal>();
		}

		Handle Subscribe(const Action& func)const {
			return Handle(self, self->subscribe(func));
		}

		void Emit(Args&&...args)const {
			self->emit(std::forward<Args...>(args...));
		}
	};
}

