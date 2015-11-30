#include "GameState.h"

GameState::GameState( const glm::uvec2& size, size_t nrOfTeams, size_t snakesPerTeam, size_t snakeLength, size_t nrOfApples ) {
	// Make sure that the input doesn't cause problems.
	assert( 0 < size.x					);
	assert( 0 < size.y					);
	assert( 0 < nrOfTeams				);
	assert( 0 < snakesPerTeam			);
	assert( 1 < snakeLength				);

	this->Size					= size;

	// Initialize the game board.
	this->Board.resize( this->Size.y );
	for ( size_t y = 0; y < this->Size.y; ++y ) {
		this->Board[y].resize( this->Size.x );
		for ( size_t x = 0; x < this->Size.x; ++x ) {
			this->Board[y][x]				= 0;					// Mark each tile on the game board as free.	// TODO: Remove magic number
		}
	}

	// Initialize teams and snakes.
	this->Teams.resize( nrOfTeams );
	for ( auto team_it = this->Teams.begin(); team_it != this->Teams.end(); ++team_it ) {

		// Initialize the snakes.
		team_it->Snakes.resize( snakesPerTeam );
		for ( auto snake_it = team_it->Snakes.begin(); snake_it != team_it->Snakes.end(); ++snake_it ) {
			glm::uvec2						pos( 5, 5 );			// Arbitrary spawn position.					// TODO: Remove magic number
			this->Board[pos.y][pos.x]		= 1;					// Block the heads position in the board.		// TODO: Remove magic number
			snake_it->SegmentsToSpawn		= snakeLength - 1;		// Only the snakes head is on the board at the start, rest of the body gets spawned later.
			snake_it->Segments.push_back( pos );
		}
	}

	// Initialize apples.
	this->Apples.resize( nrOfApples );
	for ( auto& apple_it : this->Apples ) {
		this->SpawnApple( apple_it );
	}
}

void GameState::SpawnApple( glm::uvec2& apple ) {
	do {
		apple.x		= rand() % this->Size.x;
		apple.y		= rand() % this->Size.y;
	} while ( this->Board[apple.y][apple.x] != 0 );

	this->Board[apple.y][apple.x] = 2;
}