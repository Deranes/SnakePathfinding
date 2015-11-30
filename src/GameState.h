#pragma once

#include <glm/vec2.hpp>
#include <vector>

enum class Move {
	Right,
	Left,
	Down,
	Up
};

struct Snake {
	std::vector<glm::uvec2>			Segments;
	size_t							SegmentsToSpawn;			// Number of segments that the snake should grow.
	bool							Dead						= false;
};

struct Team {
	std::vector<Snake>				Snakes;
};

class GameState {
public:
										GameState					( const glm::uvec2& size, size_t nrOfTeams, size_t snakesPerTeam, size_t snakeLength, size_t nrOfApples );
	
	void								SpawnApple					( glm::uvec2& apple );
										
										glm::uvec2							Size;						// Size of the game board.
	std::vector<std::vector<size_t>>	Board;						// The game board				// TODO: Fix "magic" datatype.
	std::vector<Team>					Teams;
	std::vector<glm::uvec2>				Apples;						// Positions of the apples.
};
