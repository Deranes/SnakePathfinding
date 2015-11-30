#include "Human.h"

#include <SFML/Graphics.hpp>

#define IsKeyPressed( x )	{ sf::Keyboard::( sf::Keyboard::Key::x ); }

void MoveIfKeyPressed( sf::Keyboard::Key key1, sf::Keyboard::Key key2, Move conditionalMove, Move& outMove ) {
	if ( sf::Keyboard::isKeyPressed( key1 ) || sf::Keyboard::isKeyPressed( key2 ) ) {
		outMove = conditionalMove;
	}
}

void Human::MakeMoves( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) {
	for ( size_t i = 0; i < outMoves.size(); ++i ) {
		MoveIfKeyPressed( sf::Keyboard::Key::Left,	sf::Keyboard::Key::A, Move::Left,	outMoves[i] );
		MoveIfKeyPressed( sf::Keyboard::Key::Right,	sf::Keyboard::Key::D, Move::Right,	outMoves[i] );
		MoveIfKeyPressed( sf::Keyboard::Key::Up,	sf::Keyboard::Key::W, Move::Up,		outMoves[i] );
		MoveIfKeyPressed( sf::Keyboard::Key::Down,	sf::Keyboard::Key::S, Move::Down,	outMoves[i] );
	}
}
