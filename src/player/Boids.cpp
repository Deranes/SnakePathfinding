#include "Boids.h"

#include <glm/geometric.hpp>

#define RULE_FACTOR_COHESION			0.7f
#define RULE_FACTOR_ALIGNMENT			0.5f
#define RULE_FACTOR_GOAL				1.5f
#define RULE_FACTOR_LOCAL_GOAL			7.0f
#define RULE_FACTOR_SEPERATION			9.0f
#define RULE_FACTOR_TEAM_SEPERATION		16.0f
#define AVOIDANCE_DISTANCE				2			// Detection distance for seperating snakes from blocked tiles.
#define TEAM_AVOIDANCE_DISTANCE			4.0f		// Detection distance for seperating snakes from snakes in the same team.
#define LOCAL_GOAL_DISTANCE				4.0f		// Detection distance for individual snakes grabbing nearby apples.

Move ChooseSafeMove( const glm::vec2& direction, Move previousMove, const std::vector<Move>& safeMoves );

void Boids::MakeMoves( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) {
	const Team& team						= currentState.Teams[teamIndex];
	const glm::vec2 teamAvaragePosition		= CalculateAvaragePosition( currentState, teamIndex );
	
	// Choose a new apple for the team as its goal if the previous goal-apple was taken.
	if ( !currentState.IsTileWalkable( m_GoalTile ) || currentState.Board[m_GoalTile.y][m_GoalTile.x] != Tile::Apple ) {
		m_GoalTile		= currentState.FindClosestApple( teamAvaragePosition );		// TODO: Figure out another goal if there are no apples.
	}

	// Decide for each snake which direction it should move.
	for ( size_t snakeIndex = 0; snakeIndex < team.Snakes.size(); ++snakeIndex ) {
		const Snake& snake						= team.Snakes[snakeIndex];
		const glm::ivec2& snakeTile				= snake.Segments[0];
		const glm::vec2 snakePosition			= glm::vec2( snakeTile );

		// Calculate which moves the snake can make without dying this turn.		// TODO: Take into account that tails move.
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

		// If there is only one safe move it is chosen, and we continue to the next snake instead.
		if ( safeMoves.size() == 1 ) {
			outMoves[snakeIndex]		= safeMoves.front();
			continue;
		}

		// Calculate and accumulate the effect of each rule that makes up boids (plus some extra ones specialized for the application snake/nibbles).
		glm::vec2 newSnakeDirection		= glm::vec2( 0.0f );
		newSnakeDirection				+= RULE_FACTOR_GOAL					* GoalDirection( currentState, teamIndex, snakeIndex );
		newSnakeDirection				+= RULE_FACTOR_LOCAL_GOAL			* LocalGoalDirection( currentState, teamIndex, snakeIndex );
		newSnakeDirection				+= RULE_FACTOR_SEPERATION			* SeperationDirection( currentState, teamIndex, snakeIndex );
		if ( team.Snakes.size() > 1 ) {		// Only apply team-rules if the snake is not alone in the team.
			newSnakeDirection				+= RULE_FACTOR_COHESION				* CohesionDirection( currentState, teamIndex, snakeIndex );
			newSnakeDirection				+= RULE_FACTOR_ALIGNMENT			* AlignmentDirection( currentState, teamIndex, snakeIndex );
			newSnakeDirection				+= RULE_FACTOR_TEAM_SEPERATION		* TeamSeperationDirection( currentState, teamIndex, snakeIndex );
		}

		outMoves[snakeIndex]		= ChooseSafeMove( newSnakeDirection, outMoves[snakeIndex], safeMoves );
	}
}

glm::vec2 Boids::CalculateAvaragePosition( const GameState & gameState, const size_t teamIndex ) const {
	const Team& team					= gameState.Teams[teamIndex];
	glm::ivec2 summedTeamPosition		= glm::ivec2( 0 );
	for ( const auto& snake : team.Snakes ) {
		summedTeamPosition		+= snake.Segments[0];
	}
	return glm::vec2( summedTeamPosition ) / static_cast<float>(team.Snakes.size());
}

glm::vec2 Boids::CohesionDirection( const GameState & gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team					= gameState.Teams[teamIndex];
	const Snake& snake					= team.Snakes[snakeIndex];
	const glm::vec2& snakePosition		= snake.Segments[0];

	// Calculate avarage position of all other snakes on the team.
	glm::ivec2 summedTeamPosition		= glm::ivec2( 0 );
	for ( size_t i = 0; i < team.Snakes.size(); ++i ) {
		// Skip self.
		if ( i == snakeIndex ) {
			continue;
		}

		summedTeamPosition		+= team.Snakes[i].Segments[0];
	}
	const glm::vec2 teamPosition		= glm::vec2( summedTeamPosition ) / static_cast<float>(team.Snakes.size());

	// Calculate normalized direction to the avarage position of the team.
	const glm::vec2 vectorToTeamPosition		= teamPosition - snakePosition;
	if ( vectorToTeamPosition != glm::vec2( 0.0f ) ) {
		return glm::normalize( vectorToTeamPosition );
	}
	return glm::vec2( 0.0f );
}

glm::vec2 Boids::AlignmentDirection( const GameState & gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team		= gameState.Teams[teamIndex];

	// Calculate avarage direction of all other snakes on the team.
	glm::vec2 direction		= glm::vec2( 0.0f );
	for ( size_t i = 0; i < team.Snakes.size(); ++i ) {
		const Snake& snake		= team.Snakes[i];

		// Skip self.
		if ( i == snakeIndex ) {
			continue;
		}
		
		if ( snake.Segments.size() >= 2 ) {									// Avoids crash from accessing segment[1] in the snakes when calculating their individual direction.
			direction		+= snake.Segments[0] - snake.Segments[1];		// The difference in position of the first two segments should give the direction the snake moved the previous frame.
		}
	}

	if ( direction != glm::vec2( 0.0f ) ) {
		return glm::normalize( direction );
	}
	return glm::vec2( 0.0f );
}

glm::vec2 Boids::GoalDirection( const GameState & gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team				= gameState.Teams[teamIndex];
	const Snake& snake				= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile		= snake.Segments[0];
	if ( m_GoalTile != snakeTile ) {
		return glm::normalize( glm::vec2( m_GoalTile - snakeTile ) );	// Calculate normalized direction towards goal.
	}
	return glm::vec2( 0.0f );
}

glm::vec2 Boids::LocalGoalDirection( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team					= gameState.Teams[teamIndex];
	const Snake& snake					= team.Snakes[snakeIndex];
	const glm::vec2& snakePosition		= snake.Segments[0];

	// Find and calculate distance to closest apple.
	const glm::ivec2 closestApple				= gameState.FindClosestApple( snakePosition );
	const glm::vec2 vectorToClosestApple		= glm::vec2( closestApple ) - snakePosition;
	const float distanceToClosestAppleSqrd		= glm::dot( vectorToClosestApple, vectorToClosestApple );

	// Return direction to closest apple if it is within detection distance.
	if ( distanceToClosestAppleSqrd <= LOCAL_GOAL_DISTANCE * LOCAL_GOAL_DISTANCE ) {
		if ( distanceToClosestAppleSqrd != 0.0f ) {
			return vectorToClosestApple / distanceToClosestAppleSqrd;		// Direction to the local goal (closest apple), diminishes with distance.
		}
	}
	return glm::vec2( 0.0f );	// Apple not found (or on the same tile as the snake for some reason).
}

glm::vec2 Boids::SeperationDirection( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team				= gameState.Teams[teamIndex];
	const Snake& snake				= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile		= snake.Segments[0];

	// Accumulate repelling forces that keeps the snake away from blocked tiles within the avoidance distance.
	glm::vec2 avoidDirection		= glm::vec2( 0.0f );
	for ( int dy = -AVOIDANCE_DISTANCE; dy <= AVOIDANCE_DISTANCE; ++dy ) {
		for ( int dx = -AVOIDANCE_DISTANCE; dx <= AVOIDANCE_DISTANCE; ++dx ) {
			const glm::ivec2 tile		= glm::ivec2( snakeTile.x + dx, snakeTile.y + dy );

			// Tiles that are walkable are skipped, since they are safe for the snake to traverse.
			if ( gameState.IsTileWalkable( tile ) ) {		// TODO: Take into account that tails move.
				continue;
			}

			// Skip tile if it is one of the 4 first segments in the snake, since the head cannot collide with any of those segments.
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

			// Add repelling force that keeps the snake away from the tile
			const glm::vec2 vectorFromTile		= glm::vec2( snakeTile - tile );
			const float distanceFromTile		= glm::length( vectorFromTile );
			avoidDirection 						+= vectorFromTile / ( glm::pow( distanceFromTile, 3 ) );		// Force diminishes with distance.
		}
	}
	return avoidDirection;
}

glm::vec2 Boids::TeamSeperationDirection( const GameState& gameState, const size_t teamIndex, const size_t snakeIndex ) const {
	const Team& team				= gameState.Teams[teamIndex];
	const Snake& snake				= team.Snakes[snakeIndex];
	const glm::ivec2& snakeTile		= snake.Segments[0];

	// Accumulate directions/forces that keeps the snake away from the heads of other snakes in the same team.
	glm::vec2 avoidDirection		= glm::vec2( 0.0f );
	for ( size_t i = 0; i < team.Snakes.size(); ++i ) {
		// Skip self.
		if ( i == snakeIndex ) {
			continue;
		}

		const glm::vec2 vectorFromTeamMate		= snakeTile - team.Snakes[i].Segments[0];
		const float distanceFromTeamMate		= glm::length( vectorFromTeamMate );

		// Add repelling force from this snake if within the avoidance distance.
		if ( distanceFromTeamMate <= TEAM_AVOIDANCE_DISTANCE ) {
			avoidDirection		+= vectorFromTeamMate / ( glm::pow( distanceFromTeamMate, 3 ) );		// Calculate force from the individual snake, quickly diminishes with distance.
		}
	}
	return avoidDirection;		// Direction intentially not normalized so that effect varies depending on how close team-mates are.
}

Move ChooseSafeMove( const glm::vec2& direction, Move previousMove, const std::vector<Move>& safeMoves ) {
	// Choose a random move (preferably safe) if no direction is specified.
	if ( direction == glm::vec2( 0.0f ) ) {
		if ( safeMoves.empty() ) {
			return static_cast<Move>( static_cast<int>(previousMove) + 1 % 4 );
		} else {
			return safeMoves[ rand() % safeMoves.size() ];
		}
	}

	// Choose the move closest to the direction given. If that move is not safe, nullify that axis from the direction, and try choosing another move recursivly.
	if ( glm::abs( direction.x ) > glm::abs( direction.y ) ) {
		if ( direction.x > 0.0f ) {		// Direction is mostly right.
			if ( std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Right ) != safeMoves.cend() ) {
				return Move::Right;
			} else {
				const glm::vec2 modifiedDirection		= glm::vec2( 0.0f, direction.y );
				return ChooseSafeMove( modifiedDirection, previousMove, safeMoves );
			}
		} else {		// Direction is mostly left.
			if ( std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Left ) != safeMoves.cend() ) {
				return Move::Left;
			} else {
				const glm::vec2 modifiedDirection		= glm::vec2( 0.0f, direction.y );
				return ChooseSafeMove( modifiedDirection, previousMove, safeMoves );
			}
		}
	} else {		// Direction is mostly down.
		if ( direction.y > 0.0f ) {
			if ( std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Down ) != safeMoves.cend() ) {
				return Move::Down;
			} else {
				const glm::vec2 modifiedDirection		= glm::vec2( direction.x, 0.0f );
				return ChooseSafeMove( modifiedDirection, previousMove, safeMoves );
			}
		} else {		// Direction is mostly up.
			if ( std::find( safeMoves.cbegin(), safeMoves.cend(), Move::Up ) != safeMoves.cend() ) {
				return Move::Up;
			} else {
				const glm::vec2 modifiedDirection		= glm::vec2( direction.x, 0.0f );
				return ChooseSafeMove( modifiedDirection, previousMove, safeMoves );
			}
		}
	}
}
