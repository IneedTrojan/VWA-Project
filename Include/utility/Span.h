#pragma once
#include <type_traits>
#include <vector>
#include "math/PointerMath.h"
#include "math/Number.h"

namespace utility
{



	template<typename T = const void*>
	struct const_span
	{
		

		const T* begin_ptr;
		const T* end_ptr;

		const_span(const T* _start, const T* _end) : begin_ptr(_start), end_ptr(_end)
		{

		}
		const_span(const T* _start, size_t _size) : begin_ptr(_start), end_ptr(static_cast<const T*>(math::pointer_add(_start, _size)))
		{

		}
		const_span(size_t _size) : begin_ptr(reinterpret_cast<const T*>(nullptr)), end_ptr(reinterpret_cast<const T*>(math::pointer_offset(_size)))
		{

		}
		const_span(const T* _start) : const_span(_start, reinterpret_cast<const T*>(std::numeric_limits<uint64_t>::max()))
		{

		}

		const_span(const std::string& span)
			: begin_ptr(reinterpret_cast<const T*>(span.data())), end_ptr(reinterpret_cast<const T*>(span.data() + span.size()))
		{

		}
		template<typename V>
		const_span(const std::vector<V>& span)
			: begin_ptr(reinterpret_cast<const T*>(span.data())), end_ptr(reinterpret_cast<const T*>(span.data() + span.size()))
		{

		}
		/*
		const_span(T* _start, T* _end) : begin_ptr(_start), end_ptr(_end)
		{

		}
		const_span(T* _start, size_t _size) : begin_ptr(reinterpret_cast<T*>(_start)), end_ptr(math::pointer_add(_start, _size))
		{

		}
		const_span(T* _start) : const_span(_start, reinterpret_cast<T*>(std::numeric_limits<uint64_t>::max()))
		{

		}*/

		template<typename V>
		const_span(std::vector<V>& span)
			: begin_ptr(reinterpret_cast<T*>(span.data())), end_ptr(reinterpret_cast<T*>(span.data() + span.size()))
		{

		}

		template <typename U>
		operator const_span<U>() const {
			return const_span<U>(reinterpret_cast<const U*>(begin_ptr), reinterpret_cast<const U*>(end_ptr));
		}
		const_span<T> clamp(uint64_t _size)const
		{
			const uint64_t size = byteSize();
			return const_span<T>(begin_ptr, std::min(size, _size));
		}
		size_t byteSize() const
		{
			return math::pointer_distance(begin_ptr, end_ptr);
		}
		size_t offset()const
		{
			return reinterpret_cast<uint64_t>(begin_ptr);
		}

		size_t size() const requires (!std::is_void_v<T>) {
			return end_ptr - begin_ptr;
		}

		auto& operator[](size_t index) const requires (!std::is_void_v<T>) {
			return begin_ptr[index];
		}


		
		using iterator = T*;
		using const_iterator = const T*;


		const_iterator begin() const { return begin_ptr; }
		const_iterator end() const { return end_ptr; }
	};
	inline const_span<void> InfinitySpan()
	{
		return const_span<void>(std::numeric_limits<size_t>::max());
	}

	template<typename T = void*>
	struct span
	{
		T* begin_ptr;
		T* end_ptr;

		span(T* _start, T* _end): begin_ptr(_start), end_ptr(_end)
		{
			
		}
		span(T* _start, size_t _size): begin_ptr(reinterpret_cast<T*>(_start)), end_ptr(math::pointer_add(_start, _size))
		{
			
		}
		span(size_t _size) : begin_ptr(reinterpret_cast<T*>(nullptr)), end_ptr(math::pointer_offset(_size))
		{

		}
		span(T* _start): span(_start, reinterpret_cast<T*>(std::numeric_limits<uint64_t>::max()))
		{
			
		}

		template<typename V>
		span(std::vector<V>& span)
		: begin_ptr(reinterpret_cast<T*>(span.data())), end_ptr(reinterpret_cast<T*>(span.data()+span.size()))
		{

		}

		span(const T*  _start, const T* _end) = delete;
		span(const T* _start, size_t _size) = delete;
		span(const T* _start) = delete;
		template<typename V>
		span(const std::vector<V>& span) = delete;

		template <typename U>
		operator span<U>() const {
			return span<U>(reinterpret_cast<U*>(begin_ptr), reinterpret_cast<U*>(end_ptr));
		}
		operator const_span<T>()const{
			return const_span<T>(begin_ptr, end_ptr);
		}

		auto& operator[](size_t index) const requires (!std::is_void_v<T>) {
			return begin_ptr[index];
		}
		auto& operator[](size_t index)  requires (!std::is_void_v<T>) {
			return begin_ptr[index];
		}
		size_t byteSize()const
		{
			return math::pointer_distance(begin_ptr, end_ptr);
		}
		size_t size()const
		{
			return end_ptr-begin_ptr;
		}
		using iterator = T*;
		using const_iterator = const T*;

		iterator begin() { return begin_ptr; }
		iterator end() { return end_ptr; }

		const_iterator begin() const { return begin_ptr; }
		const_iterator end() const { return end_ptr; }

	};



}