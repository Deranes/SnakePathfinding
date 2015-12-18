#include "Boids.h"

#include <algorithm>
#include <glm/geometric.hpp>

#define AVOIDANCE_DISTANCE		2		// Detection distance for snake vs. blocked tile seperation.
#define LOCAL_GOAL_DISTANCE		3		// Detection distance for snake vs. apples.

glm::ivec2 ConvertMoveToIVec2( Move move ) {
	if ( move == Move::Left ) {
		return glm::ivec2( -1, 0 );
	} else if ( move == Move::Right ) {
		return glm::ivec2( 1, 0 );
	} else if ( move == Move::Up ) {
		return glm::ivec2( 0, -1 );
	} else if ( move == Move::Down ) {
		return glm::ivec2( 0, 1 );
	}
	return glm::ivec2( 0 );
}

glm::ivec2 FindClosestApple( const GameState& gameState, const glm::vec2& position ) {
	float closestDistanceSqrd		= FLT_MAX;
	glm::ivec2 closestApple			= glm::ivec2( 0 );
	for ( const auto& apple : gameState.Apples ) {
		const glm::vec2& applePosition		= glm::vec2( apple );
		glm::vec2 vectorToApple				= applePosition - position;
		const float distanceToAppleSqrd		= glm::dot( vectorToApple, vectorToApple );
		if ( distanceToAppleSqrd < closestDistanceSqrd ) {
			closestDistanceSqrd		= distanceToAppleSqrd;
			closestApple			= apple;
		}
	}
	return closestApple;
}

Move ConvertVec2ToMoveDirection( const glm::vec2& direction, Move previousMove, const std::vector<Move>& safeMoves ) {
	if ( direction == glm::vec2( 0.0f ) ) {
		if ( safeMoves.empty() ) {
			return static_cast<Move>( static_cast<int>(previousMove) + 1 % 4 );
		} else {
			return safeMoves[ rand() % safeMoves.size() ]; 
		}
	}

	if ( glm::abs( direction.x ) > glm::abs( direction.y ) ) {
		if ( direction.x > 0.0f ) {		// Direction is mostly right.
			if ( previousMove != Move::Left && std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Right ) != safeMoves.cend() ) {
				return Move::Right;
			} else {
				glm::vec2 modifiedDirection		= glm::vec2( 0.0f, direction.y );
				return ConvertVec2ToMoveDirection( modifiedDirection, previousMove, safeMoves );
			}
		} else {		// Direction is mostly left.
			if ( previousMove != Move::Right && std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Left ) != safeMoves.cend() ) {
				return Move::Left;
			} else {
				glm::vec2 modifiedDirection		= glm::vec2( 0.0f, direction.y );
				return ConvertVec2ToMoveDirection( modifiedDirection, previousMove, safeMoves );
			}
		}
	} else {		// Direction is mostly down.
		if ( direction.y > 0.0f ) {
			if ( previousMove != Move::Up && std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Down ) != safeMoves.cend() ) {
				return Move::Down;
			} else {
				glm::vec2 modifiedDirection		= glm::vec2( direction.x, 0.0f );
				return ConvertVec2ToMoveDirection( modifiedDirection, previousMove, safeMoves );
			}
		} else {		// Direction is mostly up.
			if ( previousMove != Move::Down && std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Up ) != safeMoves.cend() ) {
				return Move::Up;
			} else {
				glm::vec2 modifiedDirection		= glm::vec2( direction.x, 0.0f );
				return ConvertVec2ToMoveDirection( modifiedDirection, previousMove, safeMoves );
			}
		}
	}
}

void Boids::MakeMoves( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) {
	const Team& team						= currentState.Teams[teamIndex];
	const glm::vec2 teamAvaragePosition		= CalculateAvaragePosition( currentState, teamIndex );
	const glm::vec2 teamDirection			= AlignmentDirection( currentState, teamIndex );
	
	if ( !currentState.IsTileWalkable( m_GoalTile ) || currentState.Board[m_GoalTile.y][m_GoalTile.x] != Tile::Apple ) {
		m_GoalTile		= FindClosestApple( currentState, teamAvaragePosition );
	}

	for ( size_t snakeIndex = 0; snakeIndex < team.Snakes.size(); ++snakeIndex ) {
		const Snake& snake						= team.Snakes[snakeIndex];
		const glm::ivec2& snakeTile				= snake.Segments[0];
		const glm::vec2 snakePosition			= glm::vec2( snakeTile );

		std::vector<Move> safeMoves;
		if ( currentState.IsTileWalkable( snakeTile + glm::ivec2( 0, -1 ) ) ) {
			safeMoves.push_back( Move::Up );
		}
		if ( currentState.IsTileWalkable( snakeTile + glm::ivec2( 0, 1 ) ) ) {
			safeMoves.push_back( Move::Down );
		}
		if ( currentState.IsTileWalkable( snakeTile + glm::ivec2( -1, 0 ) ) ) {
			safeMoves.push_back( Move::Left );
		}
		if ( currentState.IsTileWalkable( snakeTile + glm::ivec2( 1, 0 ) ) ) {
			safeMoves.push_back( Move::Right );
		}
		if ( safeMoves.size() == 1 ) {
			outMoves[snakeIndex]		= safeMoves.front();
		}

		glm::vec2 newSnakeDirection		= glm::vec2( 0.0f );
		newSnakeDirection				+= 	1.5f * GoalDirection( currentState, teamIndex, snakeIndex );
		newSnakeDirection				+= 	6.0f * LocalGoalDirection( currentState, teamIndex, snakeIndex );
		newSnakeDirection				+= 10.0f * SeperationDirection( currentState, teamIndex, snakeIndex );
		if ( team.Snakes.size() > 1 ) {
			newSnakeDirection		+= 	0.7f * CohesionDirection( currentState, teamIndex, snakeIndex, teamAvaragePosition );
			newSnakeDirection		+= 	0.5f * teamDirection;
			newSnakeDirection		+= 16.0f * TeamSeperationDirection( currentState, teamIndex, snakeIndex );
		}
		outMoves[snakeIndex]		= ConvertVec2ToMoveDirection( newSnakeDirection, outMoves[snakeIndex], safeMoves );
	}
}

glm::vec2 Boids::CalculateAvaragePosition( const GameState & gameState, const size_t teamIndex ) const {
	const Team& team					= gameState.Teams[teamIndex];
	glm::ivec2 summedTeamPosition		= glm::ivec2( 0 );

	for ( const auto& snake : team.Snakes ) {
		summedTeamPosition		+= snake.Segments[0];
	}

	return glm::vec2(	summedTeamPosition.x / static_cast<float>(team.Snakes.size()),
						summedTeamPosition.y / static_cast<float>(team.Snakes.size()) );
}

glm::vec2 Boids::CohesionDirection( const GameState & gameState, const size_t teamIndex, const size_t snakeIndex, const glm::vec2& teamAvaragePosition ) const {
	const Team& team					= gameState.Teams[teamIndex];
	const Snake& snake					= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile			= snake.Segments[0];
	const glm::vec2 snakePosition		= glm::vec2( snakeTile );

	if ( team.Snakes.size() <= 1 ) {
		return glm::vec2( 0.0f );
	}

	const glm::vec2 teamAvarageSum				= teamAvaragePosition * static_cast<float>(team.Snakes.size());
	const glm::vec2 teamAvarageSelfExcluded		= ( teamAvaragePosition - snakePosition ) / static_cast<float>(team.Snakes.size() - 1);

	if ( teamAvarageSelfExcluded == snakePosition ) {
		return glm::vec2( 0.0f );
	}
	return glm::normalize( teamAvarageSelfExcluded - snakePosition );
}

glm::vec2 Boids::AlignmentDirection( const GameState & gameState, const size_t teamIndex ) const {
	const Team& team		= gameState.Teams[teamIndex];
	glm::vec2 direction		= glm::vec2( 0.0f );

	for ( auto snake : team.Snakes ) {
			direction		+= (snake.Segments.size() < 2) ? glm::vec2( 0.0f ) : snake.Segments[0] - snake.Segments[1];
	}

	if ( direction != glm::vec2( 0.0f ) ) {
		return glm::normalize( direction );
	}
	return direction;
}

glm::vec2 Boids::GoalDirection( const GameState & gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team				= gameState.Teams[teamIndex];
	const Snake& snake				= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile		= snake.Segments[0];

	if ( m_GoalTile != snakeTile ) {
		return glm::normalize( glm::vec2( m_GoalTile - snakeTile ) );
	}
	return glm::vec2( 0.0f );
}

glm::vec2 Boids::LocalGoalDirection( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team				= gameState.Teams[teamIndex];
	const Snake& snake				= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile		= snake.Segments[0];

	for ( int y = snakeTile.y - LOCAL_GOAL_DISTANCE; y <= snakeTile.y + LOCAL_GOAL_DISTANCE; ++y ) {
		for ( int x = snakeTile.x - LOCAL_GOAL_DISTANCE; x <= snakeTile.x + LOCAL_GOAL_DISTANCE; ++x ) {
			const glm::ivec2 tile		= glm::ivec2( x, y );

			if ( tile == snakeTile ) {
				continue;
			}

			if ( gameState.IsTileWalkable( tile ) && gameState.Board[tile.y][tile.x] == Tile::Apple ) {
				const glm::vec2 vectorToTile		= glm::vec2( tile - snakeTile );
				return glm::normalize( vectorToTile );
			}
		}
	}
	return glm::vec2( 0.0f );
}

glm::vec2 Boids::SeperationDirection( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team						= gameState.Teams[teamIndex];
	const Snake& snake						= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile				= snake.Segments[0];

	std::vector<glm::vec2> directions;
	directions.resize( AVOIDANCE_DISTANCE );
	for ( auto& direction : directions ) {
		direction		= glm::vec2( 0.0f );
	}

	for ( int dy = -AVOIDANCE_DISTANCE; dy <= AVOIDANCE_DISTANCE; ++dy ) {
		for ( int dx = -AVOIDANCE_DISTANCE; dx <= AVOIDANCE_DISTANCE; ++dx ) {
			const glm::ivec2 tile		= glm::ivec2( snakeTile.x + dx, snakeTile.y + dy );

			bool skipTile = false;
			for ( size_t segmentIndex = 0; segmentIndex < 4 && segmentIndex < snake.Segments.size(); ++segmentIndex ) {
				if ( tile == snake.Segments[segmentIndex] ) {
					skipTile = true;
					break;
				}
			}
			if ( skipTile ) {
				continue;
			}

			if ( !gameState.IsTileWalkable( tile ) ) {
				const glm::vec2 vectorFromTile								= glm::vec2( snakeTile - tile );
				const float distanceFromTile								= glm::length( vectorFromTile );
				directions[glm::max( glm::abs(dx), glm::abs(dy) ) - 1]		+= vectorFromTile / (distanceFromTile) * glm::max( 0.0f, 1.0f - 0.4f * ( distanceFromTile - 1.0f ) );
			}
		}
	}
	glm::vec2 avoidDirection = glm::vec2( 0.0f );
	for ( size_t i = 0; i < directions.size(); ++i ) {
		if ( directions[i] != glm::vec2( 0.0f ) ) {
			avoidDirection		+= glm::normalize( directions[i] ) / static_cast<float>(2 * i + 1);
		}
	}

	return avoidDirection;
}

glm::vec2 Boids::TeamSeperationDirection( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team				= gameState.Teams[teamIndex];
	const Snake& snake				= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile		= snake.Segments[0];

	glm::vec2 avoidDirection		= glm::vec2( 0.0f );
	for ( size_t i = 0; i < team.Snakes.size(); ++i ) {
		if ( i == snakeIndex ) {
			continue;
		}

		const glm::vec2 vectorFromTeamMate		= snakeTile - team.Snakes[i].Segments[0];
		const float distanceFromTeamMate		= glm::length( vectorFromTeamMate );

		if ( distanceFromTeamMate <= 4.0f ) {
			avoidDirection		+= vectorFromTeamMate / ( distanceFromTeamMate * distanceFromTeamMate * distanceFromTeamMate );
		}
	}
	return avoidDirection;
}
