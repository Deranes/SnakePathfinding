#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

// Pre-declerations of sfml classes to avoid exposing sfml headers to users of this class.
namespace sf {
	class Window;
	class RenderWindow;
	class Color;
	class CircleShape;
	class RectangleShape;
};

class GraphicsEngine2D {
public:
								GraphicsEngine2D			( const glm::uvec2& windowSize, const std::string& windowTitle, bool fullscreen );
								~GraphicsEngine2D			( );

	void						Clear						( );
	void						Swap						( );
	void						HandleEvents				( );
	void						DrawCircle					( const glm::vec2& position, float radius,			const glm::vec4& colour = glm::vec4( 1.0f ) );
	void						DrawRectangle				( const glm::vec2& position, const glm::vec2& size,	const glm::vec4& colour = glm::vec4( 1.0f ) );
	bool						IsWindowOpen				( ) const;
	glm::uvec2					GetWindowsSize				( ) const;
	sf::Window*					GetWindowHandle				( ) const;

private:
	sf::Color					ConvertVec4ToColor			( const glm::vec4& colour ) const;

	// Pointers since size is not known (pre-declared classes in header).
	sf::RenderWindow*			m_Window					= nullptr;
	sf::CircleShape*			m_Circle					= nullptr;
	sf::RectangleShape*			m_Rectangle					= nullptr;
};