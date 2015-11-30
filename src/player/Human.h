#pragma once

#include "Player.h"

class Human : public Player {
public:
	void						MakeMoves					( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) override;
};