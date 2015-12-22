#pragma once

#include <glm/vec2.hpp>

enum class Move {
	Up,
	Left,
	Down,
	Right
};

glm::ivec2 ConvertMoveToIVec2( Move move );