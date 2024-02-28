#pragma once
#include "EnumMath.h"
namespace graphics
{
	enum class CullFace
	{
		Front = 0,
		Back = 1,
		None = 2,
	};
	enum class TriangleMode
	{
		Triangle = 0,
		Line = 1,
		Point = 2
	};
	enum class DepthTest {
		Enabled,
		Disabled
	};
	enum class FillMode
	{
		Fill = 0,
		Line = 1
	};



}