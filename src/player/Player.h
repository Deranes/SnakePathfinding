#pragma once

#include <vector>
#include "../GameState.h"

class Player {
public:
	virtual	void				MakeMoves					( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) = 0;
};