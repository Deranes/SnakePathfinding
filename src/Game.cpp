#include "Game.h"

#include "GraphicsEngine2D.h"

#ifdef SNAKE
#include <glm/geometric.hpp>
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
#else
#include <algorithm>
#include <iostream>
#include <vector>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#endif

Game::Game() {
#ifdef SNAKE
	// Create each team and decide how they are controlled (AI-method or Human).
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_1, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_2, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_3, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );
	m_TeamDatas.push_back( TeamData( COLOUR_TEAM_4, new Boids(),		NR_OF_SNAKES_PER_TEAM ) );

	// Create the initial game state.
	m_MainState		= new GameState( glm::uvec2( GAME_BOARD_WIDTH, GAME_BOARD_HEIGHT ), m_TeamDatas.size(), NR_OF_SNAKES_PER_TEAM, SNAKE_LENGTH, NR_OF_APPLES );
#else

#endif
}

Game::~Game() {
#ifdef SNAKE
	if ( m_MainState		) { delete m_MainState;		m_MainState		= nullptr; }
#else

#endif
}

void Game::Update() {
#ifdef SNAKE
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
#else

#endif
}

void Game::Draw( GraphicsEngine2D& graphicsEngine ) {
#ifdef SNAKE
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
#else
	auto windowSize = graphicsEngine.GetWindowsSize();
	const glm::vec2 graphAreaSize			= 0.7f * glm::vec2( windowSize );
	const glm::vec2 graphAreaPosition		= 0.5f * ( glm::vec2( graphicsEngine.GetWindowsSize() ) - graphAreaSize );

	struct Range
	{
		int Min;
		int Max;

		Range( int min, int max ) : Min(min), Max(max) {};
	};

	std::vector<Range> ranges;
	ranges.push_back( Range( 0, 8 ) );
	ranges.push_back( Range( 4, 16 ) );
	ranges.push_back( Range( 8, 32 ) );
	ranges.push_back( Range( 16, 64 ) );
	ranges.push_back( Range( 32, 128 ) );

	ranges.push_back( Range( 40, 160 ) );
	ranges.push_back( Range( 40, 160 ) );
	ranges.push_back( Range( 80, 320 ) );
	ranges.push_back( Range( 0, 80 ) );
	ranges.push_back( Range( 160, 640 ) );

	ranges.push_back( Range( 0, 16 ) );
	ranges.push_back( Range( 8, 32 ) );
	ranges.push_back( Range( 16, 64 ) );
	ranges.push_back( Range( 32, 128 ) );
	ranges.push_back( Range( 64, 256 ) );

	ranges.push_back( Range( 8, 32 ) );
	ranges.push_back( Range( 8, 32 ) );
	ranges.push_back( Range( 16, 64 ) );
	ranges.push_back( Range( 0, 16 ) );
	ranges.push_back( Range( 32, 128 ) );


	ranges.push_back( Range( 0, 8 ) );
	ranges.push_back( Range( 0, 8 ) );

	int minVal = 0;
	int maxVal = 0;

	for ( auto& range : ranges )
	{
		minVal += range.Min;
		maxVal += range.Max;
	}

	auto& convolutionWithFlatOneSignal = []( const std::vector<float>& signal, int flatRange, float flatValue, std::vector<float>& out )
	{
		out.resize( signal.size() + flatRange - 1 );
		for ( int base_i = 0; base_i < out.size(); ++base_i )
		{
			float sum = 0.0f;
			for ( int i = std::max( 0, 1 + base_i - flatRange ); i <= base_i; ++i )
			{
				if ( i >= 0 && i < signal.size() ) {
					sum += signal[i];
				}
			}
			out[base_i] = flatValue * sum;
		}
	};

	size_t signalLength = 1 + ranges[0].Max - ranges[0].Min;
	std::vector<float> signal( signalLength, 1.0f / signalLength );
	for ( int i = 1; i < ranges.size(); ++i )
	{
		std::vector<float> result;
		size_t signalLength = 1 + ranges[i].Max - ranges[i].Min;
		convolutionWithFlatOneSignal( signal, signalLength, 1.0f / signalLength, result );
		signal = result;
	}

	std::vector<float>& ways = signal;

	// Draw background for where the graph is allowed
	graphicsEngine.DrawRectangle( graphAreaPosition, graphAreaSize );

	float highestWay = 0;
	for ( auto& way : ways )
	{
		if ( way > highestWay )
		{
			highestWay = way;
		}
	}

	static bool print = true;
	if ( print )
	{
		std::cout << " highestWay=" << highestWay << std::endl;
		print = false;
	}

	float barWidth = graphAreaSize.x / ways.size();
	float barYScale = 0.7f * graphAreaSize.y / highestWay;

	for ( int i = 0; i < ways.size(); ++i )
	{
		glm::vec2 barSize( barWidth, ways[i] * barYScale );
		graphicsEngine.DrawRectangle( graphAreaPosition + glm::vec2( i * barWidth, graphAreaSize.y - barSize.y ), barSize, glm::vec4( 1, 0, 0, 1 ) );
	}

	//static float prevHighest	= 1.0f;
	//float currentHighest		= 0.0f;

	//const int barWidth = 2;

	//sf::Vector2i sfMousePos		= sf::Mouse::getPosition( *graphicsEngine.GetWindowHandle() );
	//glm::vec2 mousePos			= glm::vec2( sfMousePos.x, sfMousePos.y );
	//bool useMouse				= mousePos.x >= graphAreaPosition.x && mousePos.y > graphAreaPosition.y && mousePos.x <= graphAreaPosition.x + graphAreaSize.x && mousePos.y <= graphAreaPosition.y + graphAreaSize.y;
	//float mouseFuncX			= mousePos.x - graphAreaPosition.x;

	//static int nrOfCurves = 1;

	//if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::Num0 ) ) {
	//	nrOfCurves = 0;
	//} else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::Num1 ) ) {
	//	nrOfCurves = 1;
	//} else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::Num2 ) ) {
	//	nrOfCurves = 2;
	//} else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Key::Num3 ) ) {
	//	nrOfCurves = 3;
	//};

	//static bool prevClicked = false;
	//bool thisClicked = sf::Mouse::isButtonPressed( sf::Mouse::Left );

	//if ( !prevClicked && thisClicked ) {		
	//	nrOfCurves = fminf( 3, nrOfCurves + 1 );

	//	if ( nrOfCurves == 1 ) {
	//		midPoint1 = mouseFuncX;
	//	} else if ( nrOfCurves == 2 ) {
	//		midPoint2 = mouseFuncX;
	//	} else if ( nrOfCurves == 3 ) {
	//		midPoint3 = mouseFuncX;
	//	}
	//}

	//prevClicked = thisClicked;

	//for ( int x = 0; x < graphAreaSize.x; x += barWidth ) {
	//	float coolFuncVal	= this->CoolFunction( nrOfCurves, x + 0.5f * barWidth );
	//	glm::vec2 barSize = glm::vec2( barWidth, graphAreaSize.y / prevHighest * coolFuncVal );
	//	graphicsEngine.DrawRectangle( graphAreaPosition + glm::vec2( x, graphAreaSize.y - barSize.y ), barSize, glm::vec4( 1, 0, 0, 1 ) );
	//	
	//	if ( useMouse ) {
	//		coolFuncVal		= this->CoolFunction( nrOfCurves, x + 0.5f * barWidth, useMouse, mouseFuncX );
	//		glm::vec2 barSize = glm::vec2( barWidth, graphAreaSize.y / prevHighest * coolFuncVal );
	//		graphicsEngine.DrawRectangle( graphAreaPosition + glm::vec2( x, graphAreaSize.y - barSize.y ), barSize, glm::vec4( 0, 1, 0, 0.5f ) );
	//	}

	//	if ( coolFuncVal > currentHighest ) {
	//		currentHighest = coolFuncVal;
	//	}
	//}

	//const float scale = 2.5f;
	//if ( prevHighest == 1.0f ) {
	//	prevHighest = scale * currentHighest;
	//} else if ( currentHighest > prevHighest || currentHighest < 0.1f * prevHighest ) {
	//	const float p = 0.7f;
	//	prevHighest = (1.0f-p) * prevHighest + p * ( scale * currentHighest );
	//}
	//if ( prevHighest > 0.1f ) {
	//	prevHighest = 0.1f;
	//}
#endif
}

#ifdef SNAKE
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
#else
float Game::CoolFunction( int nrOfCurves, float x, bool useMouse, float mouseX ) {
	auto normal_dist = []( float x, float centerX, float halfRange ) -> float {
		if ( x < centerX - halfRange || x > centerX + halfRange ) {
			return 0.0f;
		}

		const float e				= 2.71828f;
		const float inner_x			= 2.0f * ( x - centerX )  / halfRange;
		const float inner_x_sqrd	= inner_x * inner_x;
		const float e_power			= -0.5f * inner_x_sqrd;
		const float dividend		= powf( e, e_power );
		const float devisor			= halfRange * 1.196288344f; // 0.47725 * sqrt( 2 * pi)

		float answer				= dividend / devisor;
		return answer;
	};

	auto integrate_normal_dist = []( float lowBound, float highBound, float centerX, float halfRange ) -> float {
		if ( highBound <= lowBound ) {
			return 0.0f;
		}
		const float sqrt_2		= 1.41421356237f;
		const float a			= erff( sqrt_2 * ( highBound - centerX ) / halfRange );
		const float b			= erff( sqrt_2 * ( lowBound - centerX ) / halfRange );
		return 0.523834f * ( a - b );
	};

	auto addDist = []( float x_old, float x_new ) -> float {
		float p = 0.4f;
		return x_new < 0.0001f ? 0.0f : p * x_old + (1.0f - p) * x_new;
	};

	float range			= 400.0f;
	float ans			= 0.0f;
	float factor		= 1.0f;

	if ( nrOfCurves > 0 ) {
		factor = 1.0f;
		range /= 2.0f;
		if ( nrOfCurves > 1  ) {
			factor = integrate_normal_dist( fmaxf( midPoint1 - range, midPoint2 - range / 2.0f ), fminf( midPoint1 + range, midPoint2 + range / 2.0f ), midPoint1, range );
		} else if ( useMouse ) {
			factor = integrate_normal_dist( fmaxf( midPoint1 - range, mouseX - range / 2.0f ), fminf( midPoint1 + range, mouseX + range / 2.0f ), midPoint1, range );
		}
		if ( factor > 0.0f ) {
			ans = addDist( ans, normal_dist( x, midPoint1, range  ) / factor );
		}
		if ( nrOfCurves > 1 ) {
			factor = 1.0f;
			range /= 2.0f;
			if ( nrOfCurves > 2  ) {
				factor = integrate_normal_dist( fmaxf( midPoint2 - range, midPoint3 - range / 2.0f ), fminf( midPoint2 + range, midPoint3 + range / 2.0f ), midPoint2, range );
			} else if ( useMouse ) {
				factor = integrate_normal_dist( fmaxf( midPoint2 - range, mouseX - range / 2.0f ), fminf( midPoint2 + range, mouseX + range / 2.0f ), midPoint2, range );
			}
			if ( factor > 0.0f ) {
				ans = addDist( ans, normal_dist( x, midPoint2, range ) / factor );
			}
			if ( nrOfCurves > 2 ) {
				factor = 1.0f;
				range /= 2.0f;
				if ( useMouse ) {
					factor = integrate_normal_dist( fmaxf( midPoint3 - range, mouseX - range / 2.0f ), fminf( midPoint3 + range, mouseX + range / 2.0f ), midPoint3, range );
				}
				ans = addDist( ans, normal_dist( x, midPoint3, range ) / factor );
			}
		}
	}

	if ( !useMouse ) {
		return ans;
	}

	return addDist( ans, normal_dist( x, mouseX, range /= 2.0f ) );
}

#endif