#include "Human.h"

#include <SFML/Window/Keyboard.hpp>

#define KEY_1_MOVE_UP			sf::Keyboard::Key::Up
#define KEY_1_MOVE_LEFT			sf::Keyboard::Key::Left
#define KEY_1_MOVE_DOWN			sf::Keyboard::Key::Down
#define KEY_1_MOVE_RIGHT		sf::Keyboard::Key::Right
#define KEY_2_MOVE_UP			sf::Keyboard::Key::W
#define KEY_2_MOVE_LEFT			sf::Keyboard::Key::A
#define KEY_2_MOVE_DOWN			sf::Keyboard::Key::S
#define KEY_2_MOVE_RIGHT		sf::Keyboard::Key::D

void MoveIfKeyPressed( sf::Keyboard::Key key1, sf::Keyboard::Key key2, Move conditionalMove, Move& outMove ) {
	if ( sf::Keyboard::isKeyPressed( key1 ) || sf::Keyboard::isKeyPressed( key2 ) ) {
		outMove = conditionalMove;
	}
}

void Human::MakeMoves( const GameState currentState, size_t teamIndex, std::vector<Move>& outMoves ) {
	// Make the move of each snake in the team equal to the direction the human player presses on the keyboard.
	for ( auto& outSnakeMove : outMoves ) {
		MoveIfKeyPressed( KEY_1_MOVE_UP,		KEY_2_MOVE_UP,			Move::Up,			outSnakeMove );
		MoveIfKeyPressed( KEY_1_MOVE_LEFT,		KEY_2_MOVE_LEFT,		Move::Left,			outSnakeMove );
		MoveIfKeyPressed( KEY_1_MOVE_DOWN,		KEY_2_MOVE_DOWN,		Move::Down,			outSnakeMove );
		MoveIfKeyPressed( KEY_1_MOVE_RIGHT,		KEY_2_MOVE_RIGHT,		Move::Right,		outSnakeMove );
	}
}
