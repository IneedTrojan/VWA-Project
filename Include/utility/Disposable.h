#pragma once
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace utility
{
	class Disposable
	{
	public:
		virtual ~Disposable() = default;
	};

	class DisposableSink :public Disposable
	{
		using pair = std::pair<const std::type_info*, std::unique_ptr<Disposable>>;
		std::vector<pair> sink;
	public:
		DisposableSink() = default;

		template<typename T, typename...Args>
			requires (std::is_constructible_v<T, Args...>&& std::is_base_of_v<Disposable, T>)
		T& emplace(Args&&... args)
		{
			T* instance = new T(std::forward<Args>(args)...);
			auto identifier = &typeid(T);
			sink.emplace_back(identifier, static_cast<Disposable*>(instance));
			return *instance;
		}
		template<typename T>
			requires std::is_base_of_v<Disposable, T>
		T& get()
		{
			auto identifier = &typeid(T);
			auto it = std::ranges::find_if(sink.begin(), sink.end(), [identifier](const pair& _pair) {
				return _pair.first == identifier;
				});
			if (it == sink.end())
			{
				throw std::runtime_error("");
			}
			return reinterpret_cast<T&>(*it->second);
		}
		void clear()
		{
			sink.clear();
		}

		~DisposableSink() override = default;
	};
	

}
