#pragma once

#include <glm/vec2.hpp>
#include <string>

// Pre-declerations of sfml classes to avoid exposing sfml headers to users of this class.
namespace sf {
	class RenderWindow;
	class CircleShape;
};

class GraphicsEngine2D {
public:
								GraphicsEngine2D			( const glm::ivec2& windowSize, const std::string& windowTitle );
								~GraphicsEngine2D			( );

	void						Clear						( );
	void						Swap						( );
	void						HandleEvents				( );
	void						Draw						( );
	bool						IsWindowOpen				( ) const;

private:
	// Pointers since size is not known (pre-declared classes in header).
	sf::RenderWindow*			m_Window					= nullptr;
	sf::CircleShape*			m_Shape						= nullptr;
};