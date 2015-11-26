#include "GraphicsEngine2D.h"

int main() {
	GraphicsEngine2D graphicsEngine( glm::ivec2( 720, 360 ), "Snake pathfinding" );

	// Main game loop
	while ( true ) {
		graphicsEngine.Clear();

		graphicsEngine.HandleEvents();
		if ( !graphicsEngine.IsWindowOpen() ){
			break;
		}

		graphicsEngine.Draw();

		graphicsEngine.Swap();
	}

	return 0;	// Exit success.
};