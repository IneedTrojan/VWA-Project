#pragma once
#include <vector>

#include "math/PointerMath.h"

namespace utility
{
	class DynamicBitArray
	{
	public:

		std::vector<uint64_t> bits;
		uint64_t size;
		DynamicBitArray(uint64_t bitSetSize = 64) : size(bitSetSize)
		{
			bits.resize((bitSetSize / 64 + 1));
		}


		uint64_t next_one(uint64_t offset)const {

			uint64_t value = bits[offset / 64];

			while (value == 0 && offset < size) {
				value = bits[offset / 64];

				offset += 64;
			}
			return offset + find_positive_bit(value);
		}
		uint64_t next_zero(uint64_t offset)const {
			uint64_t value = ~bits[offset / 64];

			while (value == 0 && offset < size) {
				value = ~bits[offset / 64];

				offset += 64;
			}
			return offset + find_positive_bit(value);
		}


		static uint64_t find_positive_bit(const uint64_t value) {
			if (value == 0) {
				return 64;
			}

			uint64_t offset = 0;
			uint64_t mask = 1;

			while (offset < 64) {
				if (value & mask) {
					return offset;
				}
				mask <<= 1;
				++offset;
			}

			return 64;
		}

		void reset()
		{
			for (auto& byte : bits)
			{
				byte = 0;
			}
		}
		void setBit(uint64_t index)
		{
			bits[index / 64] |= 1ULL << (index % 64);
		}
		std::vector<std::pair<uint64_t, uint64_t>> iterator(uint64_t tolerance)const
		{
			bool lookForPositive = true;
			uint64_t offset = 0;

			std::pair<uint64_t, uint64_t> last(0, 0);

			std::vector<std::pair<uint64_t, uint64_t>> pairs;
			pairs.reserve(bits.size() * 4);

			while (offset < size)
			{
				if (lookForPositive)
				{
					offset = next_one(offset);
					last.first = offset;
					lookForPositive = false;

					offset = next_zero(offset) + 1;
					if (!pairs.empty() && offset - pairs.back().second < tolerance)
					{
						last.first = pairs.back().first;
						last.second = offset;
						pairs.pop_back();

					}
				}
				if (!lookForPositive)
				{

					lookForPositive = true;
					last.second = offset;
					pairs.push_back(last);
				}
			}
			return pairs;
		}
	};
}
