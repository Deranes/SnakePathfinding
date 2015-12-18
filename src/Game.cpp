#include "Game.h"

#include <glm/geometric.hpp>
#include "GraphicsEngine2D.h"
#include "player/Human.h"
#include "player/Boids.h"

#define GAME_BOARD_WIDTH			70
#define GAME_BOARD_HEIGHT			50
#define NR_OF_APPLES				5
#define NR_OF_SNAKES_PER_TEAM		16
#define SNAKE_LENGTH				4
#define SNAKE_GROWTH_PER_APPLE		3
#define COLOUR_TEAM_1				glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f )
#define COLOUR_TEAM_2				glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f )
#define COLOUR_TEAM_3				glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f )
#define COLOUR_TEAM_4				glm::vec4( 1.0f, 0.0f, 1.0f, 1.0f )
#define COLOUR_TEAM_5				glm::vec4( 0.0f, 1.0f, 1.0f, 1.0f )
#define COLOUR_TEAM_6				glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f )
#define COLOUR_APPLES				glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f )

Game::Game() {
	// Create each team and decide how they are controlled (AI-method or Human).
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_1, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_2, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_3, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_4, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );

	// Create the initial game state.
	m_MainState		= new GameState( glm::uvec2( GAME_BOARD_WIDTH, GAME_BOARD_HEIGHT ), m_TeamDatas.size(), NR_OF_SNAKES_PER_TEAM, SNAKE_LENGTH, NR_OF_APPLES );
}

Game::~Game() {
	if ( m_MainState		) { delete m_MainState;		m_MainState		= nullptr; }
}

void Game::Update() {
	// Get moves from all the players.
	for ( size_t teamIndex = 0; teamIndex < m_TeamDatas.size(); ++teamIndex ) {
		if ( m_MainState->Teams[teamIndex].Snakes.empty() ) {		// Check if team is dead.
			continue;		// Skip dead team.
		}
		m_TeamDatas[teamIndex].Player->MakeMoves( *m_MainState, teamIndex, m_TeamDatas[teamIndex].Moves );
	}

	// Remove the tails of the snakes.
	for ( auto& team : m_MainState->Teams ) {
		for ( auto& snake : team.Snakes ) {
			this->RemoveTail( snake );
		}
	}

	// Remove dead snakes that have had all their segments removed.
	for ( size_t snakeIndex = 0; snakeIndex < m_DeadSnakes.size(); ++snakeIndex ) {
		if ( m_DeadSnakes[snakeIndex].Segments.empty() ) {
			m_DeadSnakes.erase( m_DeadSnakes.begin() + snakeIndex );
			--snakeIndex;
		}
	}

	// Remove the tails of the dead snakes, so that they stop blocking the game board eventually.
	for ( auto& deadSnake : m_DeadSnakes ) {
		this->RemoveTail( deadSnake );
	}

	// Insert new heads onto the snakes.
	for ( size_t teamIndex = 0; teamIndex < m_MainState->Teams.size(); ++teamIndex ) {
		Team& team		= m_MainState->Teams[teamIndex];
		for ( size_t snakeIndex = 0; snakeIndex < team.Snakes.size(); ++snakeIndex ) {
			Snake& snake					= team.Snakes[snakeIndex];
			const Move move					= m_TeamDatas[teamIndex].Moves[snakeIndex];
			const glm::ivec2 movingTo		= *snake.Segments.begin() + ConvertMoveToIVec2( move );

			// Kill snake if it tries to move onto an unwalkable tile.
			if ( !m_MainState->IsTileWalkable( movingTo ) ) {
				m_DeadSnakes.push_back( snake );
				team.Snakes.erase( team.Snakes.begin() + snakeIndex );
				m_TeamDatas[teamIndex].Moves.erase( m_TeamDatas[teamIndex].Moves.begin() + snakeIndex );
				--snakeIndex;
				continue;
			}

			// Check if an apple gets eaten.
			if (  m_MainState->Board[movingTo.y][movingTo.x] == Tile::Apple ) {
				snake.SegmentsToSpawn		+= SNAKE_GROWTH_PER_APPLE;

				// Find apple that was eaten and respawn it.
				for ( auto& apple : m_MainState->Apples ) {
					if ( apple == movingTo ) {
						m_MainState->SpawnApple( apple );
					}
				}
			}

			// Insert the new head segment.
			snake.Segments.insert( snake.Segments.begin(), movingTo );
			m_MainState->Board[movingTo.y][movingTo.x]		= Tile::Blocked;		// Mark the heads new position as blocked.
		}
	}
}

void Game::Draw( GraphicsEngine2D& graphicsEngine ) {
	// Calculate screen location of the playable area.
	const glm::vec2 windowSize					= glm::vec2( graphicsEngine.GetWindowsSize() );
	const glm::vec2 scale						= glm::vec2( glm::min( windowSize.x / m_MainState->Size.x, windowSize.y / m_MainState->Size.y ) );
	const glm::vec2 playableAreaSize			= scale * glm::vec2( m_MainState->Size );
	const glm::vec2 playableAreaPosition		= 0.5f * ( glm::vec2( graphicsEngine.GetWindowsSize() ) - playableAreaSize );

	// Draw the game board.
	graphicsEngine.DrawRectangle( playableAreaPosition, playableAreaSize, glm::vec4( glm::vec3( 0.1f ), 1.0f ) );

	// Draw tiles that are blocked.
	for ( size_t y = 0; y < m_MainState->Size.y; ++y ) {
		for ( size_t x = 0; x < m_MainState->Size.x; ++x ) {
			if ( m_MainState->Board[y][x] == Tile::Blocked ) {
				graphicsEngine.DrawRectangle( playableAreaPosition + scale * glm::vec2( x, y ), scale );
			}
		}
	}

	// Draw snakes.
	for ( size_t teamIndex = 0; teamIndex < m_MainState->Teams.size(); ++teamIndex ) {
		Team& team							= m_MainState->Teams[teamIndex];
		const glm::vec4 teamHeadColour		= glm::vec4(  glm::clamp( 0.5f + glm::vec3( m_TeamDatas[teamIndex].Colour ), 0.0f, 1.0f ), 1.0f );		// Make head colour brighter.
		const glm::vec4 teamTailColour		= glm::vec4(  glm::vec3( 0.5f * m_TeamDatas[teamIndex].Colour ), 1.0f );								// Make tail colour darker.

		for ( auto snake : team.Snakes ) {
			for ( size_t segmentIndex = 0; segmentIndex < snake.Segments.size(); ++segmentIndex ) {
				const float normalizedSegmentPos		= static_cast<float>(segmentIndex) / snake.Segments.size();										// From 0 (head) to 1 (tail).
				const glm::vec4 segmentColour			= ( 1.0f - normalizedSegmentPos ) * teamHeadColour + normalizedSegmentPos * teamTailColour;		// Change colour smothly from head to tail.
				const glm::vec2 segmentPosition			= playableAreaPosition + scale * glm::vec2( snake.Segments[segmentIndex] );
				graphicsEngine.DrawRectangle( segmentPosition, scale, segmentColour );
			}
		}
	}

	// Draw Apples.
	for ( const auto& apple : m_MainState->Apples ) {
		const glm::vec2 applePosition		= playableAreaPosition + scale * ( glm::vec2( apple ) );
		graphicsEngine.DrawCircle( applePosition, 0.5f * scale.x, COLOUR_APPLES );
	}
}

void Game::RemoveTail( Snake& snake ) {
	if ( snake.SegmentsToSpawn > 0 ) {		// Don't remove tail of snake if there are segments left to spawn (e.g after eating).
		--snake.SegmentsToSpawn;
		return;
	}

	if ( snake.Segments.empty() ) {
		return;
	}

	const glm::ivec2 tailPosition							= *(snake.Segments.end() - 1);
	m_MainState->Board[tailPosition.y][tailPosition.x]		= Tile::Open;						// Mark the tails position as free.
	snake.Segments.pop_back();																	// Remove the tail of the snake.
}

glm::ivec2 Game::ConvertMoveToIVec2( Move move ) {
	if ( move == Move::Left ) {
		return glm::ivec2( -1,  0 );
	} else if ( move == Move::Right ) {
		return glm::ivec2(  1,  0 );
	} else if ( move == Move::Up ) {
		return glm::ivec2(  0, -1 );
	} else if ( move == Move::Down ) {
		return glm::ivec2(  0,  1 );
	}
	return glm::ivec2( 0 );
}
