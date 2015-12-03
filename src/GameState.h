#pragma once

#include <glm/vec2.hpp>
#include <vector>

enum class Tile {
	Open,
	Blocked,
	Apple
};

struct Snake {
	std::vector<glm::ivec2>		Segments;								// Positions of the snakes body. Starts with head and ends in the tail.
	size_t						SegmentsToSpawn				= 0;		// Number of segments that the snake should increase it's size by.
};

struct Team {
	std::vector<Snake>			Snakes;
};

class GameState {
public:
										GameState			( const glm::uvec2& size, size_t nrOfTeams, size_t snakesPerTeam, size_t snakeLength, size_t nrOfApples );
	
										// The vec2 is totally an apple, trust me.
	void								SpawnApple			( glm::ivec2& apple );

	bool								IsTileWalkable		( const glm::ivec2& tile ) const;
										
	glm::uvec2							Size;				// Size of the game board.
	std::vector<std::vector<Tile>>		Board;				// Shows the state of each tile on the game board.
	std::vector<Team>					Teams;				// Teams of snakes.
	std::vector<glm::ivec2>				Apples;				// Positions of the apples spawned.
};
