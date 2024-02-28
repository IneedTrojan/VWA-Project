#pragma once
#include <cstdint>
#include <bitset>
namespace math
{
    constexpr uint64_t SetBitsUint64(std::initializer_list<int32_t> bits)
    {
        uint64_t result = 0;
        for (const auto indx : bits)
        {
            result |= 1ull << indx;
        }
        return result;
    }
	template<size_t size>
	static size_t NextBit(std::bitset<size> bitset, size_t current = -1)
	{
		++current;
		for (size_t i = current; i < size; i++)
		{
			if (bitset[i])
			{
				return i;
			}
		}
		return size;
	}
}
