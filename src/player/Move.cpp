#include "Move.h"

glm::ivec2 ConvertMoveToIVec2( Move move ) {
	if ( move == Move::Left ) {
		return glm::ivec2( -1,  0 );
	} else if ( move == Move::Right ) {
		return glm::ivec2(  1,  0 );
	} else if ( move == Move::Up ) {
		return glm::ivec2(  0, -1 );
	} else if ( move == Move::Down ) {
		return glm::ivec2(  0,  1 );
	}
	return glm::ivec2( 0 );
}