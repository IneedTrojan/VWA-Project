#pragma once
#include <sstream>
#include <iomanip>

namespace utility
{
	template<typename datatype>
	std::string stringify_vector(const datatype* valuePtr, size_t _num_col, int decimals = 10)
	{
		std::stringstream ss;
		ss << "[";
		for (size_t x = 0; x < _num_col; x++)
		{
			ss << std::setw(decimals) << std::setprecision(decimals / 2) << static_cast<double>(valuePtr[x]);
			if (x != _num_col - 1)
			{
				ss << ", ";
			}
		}
		ss << "]";
		return ss.str();
	}

	template<typename datatype>
	std::string stringify_matrix(const datatype* valuePtr, size_t _num_col, size_t _num_row, int decimals = 10)
	{
		std::stringstream ss;

		for (size_t y = 0; y < _num_row; y++)
		{
			ss << stringify_vector<datatype>(valuePtr + (y * _num_col), _num_col, decimals);
			if (y != _num_col - 1)
			{
				ss << "\n";

			}
		}
		return ss.str();
	}

}
