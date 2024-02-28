#pragma once
namespace math
{
	template<typename T>
	inline T* pointer_sum(void* a, void* b)
	{
		return reinterpret_cast<T*>(static_cast<char*>(a) + reinterpret_cast<size_t>(b));
	}
	inline void* pointer_sum(void* a, void* b)
	{
		return static_cast<char*>(a) + reinterpret_cast<size_t>(b);
	}
	inline void* pointer_add(void* a, size_t size)
	{
		return static_cast<char*>(a) + size;
	}
	inline void* pointer_offset(size_t size)
	{
		return reinterpret_cast<char*>(size);
	}
	


	inline const void* pointer_add(const void* a, size_t size)
	{
		return static_cast<const char*>(a) + size;
	}
	
	template<typename T = size_t>
	inline T pointer_distance(const void* smaller, const void* bigger)
	{
		return static_cast<T>(reinterpret_cast<size_t>(bigger) - reinterpret_cast<size_t>(smaller));
	}
	template<typename VectorType, typename T = size_t>
	inline T vector_size(const std::vector<VectorType>& v)
	{
		return v.m_blockSize()*sizeof(T);
	}

}