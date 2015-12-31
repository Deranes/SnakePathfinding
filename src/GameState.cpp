#include "GameState.h"

#include <glm/geometric.hpp>

#define SNAKE_LENGTH_MINIMUM		2		// Minimum snake length is set to the lowest number that doesn't cause the game to crash.

GameState::GameState( const glm::uvec2& size, size_t nrOfTeams, size_t snakesPerTeam, size_t snakeLength, size_t nrOfApples ) {
	// Make sure that the input doesn't cause problems.
	assert( 0 < size.x								);
	assert( 0 < size.y								);
	assert( 0 < nrOfTeams							);
	assert( 0 < snakesPerTeam						);
	assert( SNAKE_LENGTH_MINIMUM <= snakeLength		);

	this->Size		= size;

	// Initialize the game board.
	this->Board.resize( this->Size.y );		// Create all rows
	for ( auto& row : this->Board ) {
		row.resize( this->Size.x );			// Set each rows to the specified size.
		for ( auto& tile : row ) {
			tile		= Tile::Open;		// Mark each tile as open.
		}
	}

	// Initialize teams and snakes.
	this->Teams.resize( nrOfTeams );				// Create all teams.
	for ( size_t teamIndex = 0; teamIndex < this->Teams.size(); ++teamIndex ) {
		Team& team		= this->Teams[teamIndex];

		// Initialize the snakes.
		team.Snakes.resize( snakesPerTeam );		// Create all snakes in each team.
		for ( size_t snakeIndex = 0; snakeIndex < team.Snakes.size(); ++snakeIndex ) {
			Snake& snake										= team.Snakes[snakeIndex];
			snake.SegmentsToSpawn								= snakeLength - 1;									// Only the snakes head is on the board at the start, rest of the body gets spawned later.
			const glm::ivec2 spawnPosition						= glm::ivec2(	5 + snakeIndex * 3,					// Arbitrary spawn position.					// TODO: Revamp spawn positions.
																				5 + teamIndex * 10 );					
			this->Board[spawnPosition.y][spawnPosition.x]		= Tile::Blocked;									// Block the snakes position in the board.
			snake.Segments.push_back( spawnPosition );
		}
	}

	// Initialize apples.
	this->Apples.resize( nrOfApples );		// Create all apples.
	for ( auto& apple : this->Apples ) {
		this->SpawnApple( apple );
	}
}

void GameState::SpawnApple( glm::ivec2& apple ) {
	// Randomize a spawn location for the apple until an open position is found.
	do {
		apple.x		= rand() % this->Size.x;
		apple.y		= rand() % this->Size.y;
	} while ( this->Board[apple.y][apple.x] != Tile::Open );

	this->Board[apple.y][apple.x]		= Tile::Apple;		// Block the tile so that other apples can't spawn on it.
}

bool GameState::IsTileWalkable( const glm::ivec2& tile ) const {
	if ( tile.x < 0 || tile.y < 0 || static_cast<size_t>(tile.x) >= this->Size.x || static_cast<size_t>(tile.y) >= this->Size.y ) {		// Check collision against walls.
		return false;
	}
	return this->Board[tile.y][tile.x] != Tile::Blocked;		// Check if destination tile is blocked.
}

glm::ivec2 GameState::FindClosestApple( const glm::vec2& position ) const {
	glm::ivec2 closestApple			= position;		// Arbitrary initial value, will be overwritten if any apples exist.
	float closestDistanceSqrd		= FLT_MAX;		// Initial value chosen so that the first apple will overwrite it.

	for ( const auto& apple : this->Apples ) {
		// Calculate distance (squared) to the apple.
		const glm::vec2 vectorToApple		= glm::vec2( apple ) - position;
		const float distanceToAppleSqrd		= glm::dot( vectorToApple, vectorToApple );

		// Save information about the apple if it is closer than the previously closest apple.
		if ( distanceToAppleSqrd < closestDistanceSqrd ) {
			closestDistanceSqrd		= distanceToAppleSqrd;
			closestApple			= apple;
		}
	}
	return closestApple;
}
