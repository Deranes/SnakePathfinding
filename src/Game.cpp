#include "Game.h"

#include <chrono>
#include <glm/geometric.hpp>
#include <thread>
#include "GraphicsEngine2D.h"
#include "player/Human.h"

#define NR_OF_SNAKES_PER_TEAM		1
#define SNAKE_LENGTH				4
#define NR_OF_APPLES				5
#define SNAKE_GROWTH_PER_APPLE		3

Game::Game() {
	m_TeamDatas.push_back( TeamData( glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f ), new Human(), NR_OF_SNAKES_PER_TEAM ) );
	m_MainState		= new GameState( glm::uvec2( 40, 30 ), m_TeamDatas.size(), NR_OF_SNAKES_PER_TEAM, SNAKE_LENGTH, NR_OF_APPLES );
}

Game::~Game() {
	if ( m_MainState		) { delete m_MainState;		m_MainState = nullptr;		}
}

void Game::Update() {
	// Get moves from the artificial intelligences.
	for ( size_t i = 0; i < m_TeamDatas.size(); ++i ) {
		m_TeamDatas[i].Player->MakeMoves( *m_MainState, i, m_TeamDatas[i].Moves );
	}

	// Remove the tails of the snakes.
	for ( auto team_it = m_MainState->Teams.begin(); team_it != m_MainState->Teams.end(); ++team_it ) {
		for ( auto snake_it = team_it->Snakes.begin(); snake_it != team_it->Snakes.end(); ++snake_it ) {
			if ( snake_it->SegmentsToSpawn > 0 && !snake_it->Dead ) {				// Don't remove tail if there are segments left to spawn (e.g after eating).
				--snake_it->SegmentsToSpawn;
			} else if ( snake_it->Segments.size() ) {
				const glm::uvec2 tailPosition		= *(snake_it->Segments.end() - 1);
				m_MainState->Board[tailPosition.y][tailPosition.x]		= 0;		// Mark the tails previous position as free.
				snake_it->Segments.pop_back();
			}
		}
	}

	// Insert new heads onto the snakes.
	for ( size_t teamIndex = 0; teamIndex < m_MainState->Teams.size(); ++teamIndex ) {
		Team& team		= m_MainState->Teams[teamIndex];
		for ( size_t snakeIndex = 0; snakeIndex < team.Snakes.size(); ++snakeIndex ) {
			Snake& snake		= team.Snakes[snakeIndex];
			const Move move		= m_TeamDatas[teamIndex].Moves[snakeIndex];

			if ( snake.Dead ) {
				continue;
			}

			glm::uvec2 moveDirection( 0 );
			if ( move == Move::Left ) {
				moveDirection		= glm::uvec2( -1,  0 );
			} else if ( move == Move::Right ) {
				moveDirection		= glm::uvec2(  1,  0 );
			} else if ( move == Move::Up ) {
				moveDirection		= glm::uvec2(  0, -1 );
			} else if ( move == Move::Down ) {
				moveDirection		= glm::uvec2(  0,  1 );
			}

			const glm::uvec2 movingTo		= *snake.Segments.begin() + moveDirection;

			{
				bool collision		= movingTo.x < 0 || movingTo.y < 0 || movingTo.x >= m_MainState->Size.x || movingTo.y >= m_MainState->Size.y;	// Check collision against walls.
				collision = collision ? collision : m_MainState->Board[movingTo.y][movingTo.x] == 1;												// Check if destination tile is blocked.
				if ( collision ) {
					snake.Dead		= true;
					continue;
				}

				// Check if an apple gets eaten.
				if (  m_MainState->Board[movingTo.y][movingTo.x] == 2 ) {
					snake.SegmentsToSpawn += SNAKE_GROWTH_PER_APPLE;

					// Find apple that was eaten and respawn it.
					for ( auto& apple : m_MainState->Apples ) {
						if ( apple == movingTo ) {
							m_MainState->SpawnApple( apple );
						}
					}
				}
			}

			snake.Segments.insert( snake.Segments.begin(), movingTo );
			m_MainState->Board[movingTo.y][movingTo.x]		= 1;			// Mark the heads new position as blocked.
		}
	}

	std::this_thread::sleep_for( std::chrono::milliseconds( 85 ) );
}

void Game::Draw( GraphicsEngine2D& graphicsEngine ) {
	// Calculate screen location of the playable area.
	const glm::vec2 windowSize					= glm::vec2( graphicsEngine.GetWindowsSize() );
	const glm::vec2 scale						= glm::vec2( glm::min( windowSize.x / m_MainState->Size.x, windowSize.y / m_MainState->Size.y ) );
	const glm::vec2 playableAreaSize			= scale * static_cast<glm::vec2>(m_MainState->Size);
	const glm::vec2 playableAreaPosition		= 0.5f * ( static_cast<glm::vec2>(graphicsEngine.GetWindowsSize()) - playableAreaSize );

	// Draw the game board.
	graphicsEngine.DrawRectangle( playableAreaPosition, playableAreaSize, glm::vec4( glm::vec3( 0.1f ), 1.0f ) );

	// Draw tiles that are not walkable.
	for ( size_t y = 0; y < m_MainState->Size.y; ++y ) {
		for ( size_t x = 0; x < m_MainState->Size.x; ++x ) {
			if ( m_MainState->Board[y][x] == 1 ) {
				graphicsEngine.DrawRectangle( playableAreaPosition + scale * glm::vec2( x, y ), scale );
			}
		}
	}

	// Draw snakes.
	for ( size_t teamIndex = 0; teamIndex < m_MainState->Teams.size(); ++teamIndex ) {
		Team& team							= m_MainState->Teams[teamIndex];
		const glm::vec4 teamHeadColour		= glm::vec4(  glm::clamp( 0.5f + glm::vec3( m_TeamDatas[teamIndex].Colour ), 0.0f, 1.0f ), 1.0f );		// Make head colour brighter.
		const glm::vec4 teamTailColour		= glm::vec4(  glm::vec3( 0.5f * m_TeamDatas[teamIndex].Colour ), 1.0f );								// Make tail colour darker.

		for ( auto snake_it : team.Snakes ) {
			if ( snake_it.Dead ) {
				continue;
			}
			for ( size_t i = 0; i < snake_it.Segments.size(); ++i ) {
				const float normalizedSegmentPos	= static_cast<float>(i) / snake_it.Segments.size();
				const glm::vec4 segmentColour		= ( 1.0f - normalizedSegmentPos ) * teamHeadColour + normalizedSegmentPos * teamTailColour;		// Change colour smothly from head to tail.
				const glm::vec2 segmentPosition		= playableAreaPosition + scale * glm::vec2( snake_it.Segments[i] );
				graphicsEngine.DrawRectangle( segmentPosition, scale, segmentColour );
			}
		}
	}

	// Draw Apples.
	for ( const auto& apple_it : m_MainState->Apples ) {
		const glm::vec2 applePosition		= playableAreaPosition + scale * ( glm::vec2( apple_it ) );
		graphicsEngine.DrawCircle( applePosition, 0.5f * scale.x, glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	}
}
