#include "GraphicsEngine2D.h"

#include <SFML/Graphics.hpp>

GraphicsEngine2D::GraphicsEngine2D( const glm::uvec2& windowSize, const std::string& windowTitle ) {
	m_Window			= new sf::RenderWindow( sf::VideoMode( windowSize.x, windowSize.y ), windowTitle );
	m_Rectangle			= new sf::RectangleShape();
	m_Circle			= new sf::CircleShape();
}

GraphicsEngine2D::~GraphicsEngine2D() {
	if ( m_Window		) { delete m_Window;		m_Window		= nullptr;	}
	if ( m_Rectangle	) { delete m_Rectangle;		m_Rectangle		= nullptr;	}
	if ( m_Circle		) { delete m_Circle;		m_Circle		= nullptr;	}
}

void GraphicsEngine2D::Clear() {
	m_Window->clear();
}

void GraphicsEngine2D::Swap() {
	m_Window->display();
}

void GraphicsEngine2D::HandleEvents() {
	sf::Event event;
	while ( m_Window->pollEvent( event ) ) {
		if ( event.type == sf::Event::Closed ) {
			m_Window->close();
		}
	}
}

void GraphicsEngine2D::DrawRectangle( const glm::vec2& position, const glm::vec2& size, const glm::vec4& colour ) {
	m_Rectangle->setPosition	(	sf::Vector2f( position.x,		position.y		) );
	m_Rectangle->setSize		(	sf::Vector2f( size.x,			size.y			) );
	m_Rectangle->setFillColor	(	sf::Color(	static_cast<sf::Uint8>( 255.0f * colour.r ),
												static_cast<sf::Uint8>( 255.0f * colour.g ),
												static_cast<sf::Uint8>( 255.0f * colour.b ),
												static_cast<sf::Uint8>( 255.0f * colour.a ) ) );
	m_Window->draw( *m_Rectangle );
}

void GraphicsEngine2D::DrawCircle ( const glm::vec2& position, float radius,			const glm::vec4& colour ) {
	m_Circle->setPosition		(	sf::Vector2f( position.x,		position.y		) );
	m_Circle->setRadius			(	radius	);
	m_Circle->setFillColor		(	sf::Color(	static_cast<sf::Uint8>( 255.0f * colour.r ),
												static_cast<sf::Uint8>( 255.0f * colour.g ),
												static_cast<sf::Uint8>( 255.0f * colour.b ),
												static_cast<sf::Uint8>( 255.0f * colour.a ) ) );
	m_Window->draw( *m_Circle );
}

bool GraphicsEngine2D::IsWindowOpen() const {
	return m_Window->isOpen();
}

glm::uvec2 GraphicsEngine2D::GetWindowsSize() const {
	return glm::uvec2( m_Window->getSize().x, m_Window->getSize().y );
}
