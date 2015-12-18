#include <chrono>
#include <SFML/Window/Keyboard.hpp>
#include <thread>
#include "Game.h"
#include "GraphicsEngine2D.h"

#define WINDOW_RESOLUTION_WIDTH			1920	
#define WINDOW_RESOLUTION_HEIGHT		1080
#define WINDOW_TITLE					"Snake pathfinding"
#define KEY_GAME_EXIT					sf::Keyboard::Key::Escape
#define KEY_GAME_RESET					sf::Keyboard::Key::R

int main() {
	GraphicsEngine2D graphicsEngine( glm::uvec2( WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT ), WINDOW_TITLE );
	Game game;

	// Main game loop
	while ( true ) {
		graphicsEngine.Clear();

		graphicsEngine.HandleEvents();
		if ( !graphicsEngine.IsWindowOpen() || sf::Keyboard::isKeyPressed( KEY_GAME_EXIT ) ){
			break;
		}

		// Reset the game if requested by the user.
		if ( sf::Keyboard::isKeyPressed( KEY_GAME_RESET ) ) {
			new (&game)Game();		// Recreates the game.
		}

		game.Update();
		game.Draw( graphicsEngine );

		// Slow down the game so that it is possible to see what is going on.
		std::this_thread::sleep_for( std::chrono::milliseconds( 70 ) );		// TODO: Sleep shorter if the frame is longer.

		graphicsEngine.Swap();
	}

	return 0;	// Exit success.
};