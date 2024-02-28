#pragma once
#include <bitset>
#include <cstdint>

namespace renderer {
	struct LayerMask
	{
		std::bitset<64> layerMask;

		LayerMask(std::bitset<64> mask) {

		}
		LayerMask(): layerMask(0) {

		}
		operator std::bitset<64>()const {
			return layerMask;
		}

		bool Collides(LayerMask other)const {
			return (layerMask & other.layerMask).to_ullong();
		}
	};
}
