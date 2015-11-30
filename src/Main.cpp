#include "GraphicsEngine2D.h"
#include "Game.h"

int main() {
	GraphicsEngine2D graphicsEngine( glm::uvec2( 720, 360 ), "Snake pathfinding" );
	Game game;

	// Main game loop
	while ( true ) {
		graphicsEngine.Clear();

		graphicsEngine.HandleEvents();
		if ( !graphicsEngine.IsWindowOpen() ){
			break;
		}

		game.Update();
		game.Draw( graphicsEngine );

		graphicsEngine.Swap();
	}

	return 0;	// Exit success.
};