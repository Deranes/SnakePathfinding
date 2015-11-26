#include "GraphicsEngine2D.h"

#include <SFML/Graphics.hpp>

GraphicsEngine2D::GraphicsEngine2D( const glm::ivec2& windowSize, const std::string& windowTitle ) {
	m_Window			= new sf::RenderWindow( sf::VideoMode( windowSize.x, windowSize.y ), windowTitle );
	m_Shape				= new sf::CircleShape( 100.f );
	m_Shape->setFillColor( sf::Color::Green );
}

GraphicsEngine2D::~GraphicsEngine2D() {
	if ( m_Window		) { delete m_Window;		m_Window = nullptr;		}
	if ( m_Shape		) { delete m_Shape;			m_Shape = nullptr;		}
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

void GraphicsEngine2D::Draw() {
	m_Window->draw( *m_Shape );
}

bool GraphicsEngine2D::IsWindowOpen() const {
	return m_Window->isOpen();
}