#pragma once

//#define SNAKE

#ifdef SNAKE
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>
#include "GameState.h"
#else

#endif

class		GraphicsEngine2D;
#ifdef SNAKE
class		Player;
enum class	Move;
#else

#endif

#ifdef SNAKE
struct TeamData {
	TeamData( const glm::vec4& colour, Player* player, size_t nrOfSnakes ) {
		this->Colour		= glm::clamp( colour, 0.0f, 1.0f );
		this->Player		= player;
		Moves.resize( nrOfSnakes );
	}
	glm::vec4				Colour;
	Player*					Player;
	std::vector<Move>		Moves;
};
#endif

class Game {
public:
								Game					( );
								~Game					( );
	void						Update					( );
	void						Draw					( GraphicsEngine2D& graphicsEngine );

private:
#ifdef SNAKE
	void						RemoveTail				( Snake& snake );

	GameState*					m_MainState				= nullptr;
	std::vector<TeamData>		m_TeamDatas;
	std::vector<Snake>			m_DeadSnakes;
#else
	float						CoolFunction( int nrOfCurves, float x, bool useMouse = false, float mouseX = 0.0f );

	float						midPoint1		= 375.0f;
	float						midPoint2		= 400.0f;
	float						midPoint3		= 350.0f;
#endif
};
