/*  ----------------------------------- INFOS
	Texture related types and function for use with the rendering API.
	
*/

#ifndef _TEXTUREH_
#define _TEXTUREH_

#include "../render/atlas.h"

internal v2 texture_convertTexposMinToNDC(rtpAtlasSprite sprite) {
	return { sprite.positionX / 1024.f, sprite.positionY / 1024.f};
};

internal v2 texture_convertTexposMaxToNDC(rtpAtlasSprite sprite) {
	return { (sprite.positionX + sprite.sourceWidth) / 1024.f, (sprite.positionY + sprite.sourceHeight) / 1024.f};
};


#endif /* _TEXTUREH_ */