#pragma once

#include "Player.h"

class Boids : public Player {
public:
	void			MakeMoves						( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) override;

private:
	glm::vec2		CalculateAvaragePosition		( const GameState& gameState, const size_t teamIndex ) const;
	glm::vec2		CohesionDirection				( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex, const glm::vec2& teamAvaragePosition ) const;
	glm::vec2		AlignmentDirection				( const GameState& gameState, const size_t teamIndex ) const;
	glm::vec2		GoalDirection					( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const;
	glm::vec2		LocalGoalDirection				( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const;
	glm::vec2		SeperationDirection				( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const;
	glm::vec2		TeamSeperationDirection			( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const;

	glm::ivec2		m_GoalTile						= glm::ivec2( -1 );		// Position chosen so that goal gets recalculated first time moves are calculated.
};