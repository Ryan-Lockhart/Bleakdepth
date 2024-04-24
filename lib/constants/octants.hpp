#pragma once

#include "octant.hpp"

namespace Bleakdepth {
	constexpr const octant_t<i32> Octants[] {
		{  0,  1,  1,  0 },
		{  1,  0,  0,  1 },
		{  0, -1,  1,  0 },
		{ -1,  0,  0,  1 },
		{  0, -1, -1,  0 },
		{ -1,  0,  0, -1 },
		{  0,  1, -1,  0 },
		{  1,  0,  0, -1 }
	};
}