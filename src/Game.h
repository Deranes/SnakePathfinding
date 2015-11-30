#pragma once

#include <glm/vec4.hpp>
#include <glm/geometric.hpp>
#include "GameState.h"

class		GraphicsEngine2D;
class		Player;
enum class	Move;

struct TeamData {
	TeamData( const glm::vec4& colour, Player* player, size_t nrOfSnakes ) {
		this->Colour				= glm::clamp( colour, 0.0f, 1.0f );
		this->Player				= player;
		Moves.resize( nrOfSnakes );
	}
	glm::vec4					Colour;
	Player*						Player;
	std::vector<Move>			Moves;
};

class Game {
public:
								Game						( );
								~Game						( );
	void						Update						( );
	void						Draw						( GraphicsEngine2D& graphicsEngine );

private:
	GameState*					m_MainState					= nullptr;
	std::vector<TeamData>		m_TeamDatas;
};
