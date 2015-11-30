#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

// Pre-declerations of sfml classes to avoid exposing sfml headers to users of this class.
namespace sf {
	class RenderWindow;
	class RectangleShape;
};

class GraphicsEngine2D {
public:
								GraphicsEngine2D			( const glm::uvec2& windowSize, const std::string& windowTitle );
								~GraphicsEngine2D			( );

	void						Clear						( );
	void						Swap						( );
	void						HandleEvents				( );
	void						DrawRectangle				( const glm::vec2& position, const glm::vec2& size, const glm::vec4& colour = glm::vec4( 1.0f ) );
	bool						IsWindowOpen				( ) const;
	glm::uvec2					GetWindowsSize				( ) const;

private:
	// Pointers since size is not known (pre-declared classes in header).
	sf::RenderWindow*			m_Window					= nullptr;
	sf::RectangleShape*			m_Rectangle					= nullptr;
};