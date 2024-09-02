/*  ----------------------------------- INFOS
	Texture related types and function for use with the rendering API.
	
*/

internal v2 texture_convertTexposMinToNDC(rtpAtlasSprite sprite) {
	return { sprite.positionX / 1024.f, sprite.positionY / 1024.f};
};

internal v2 texture_convertTexposMaxToNDC(rtpAtlasSprite sprite) {
	return { (sprite.positionX + sprite.sourceWidth) / 1024.f, (sprite.positionY + sprite.sourceHeight) / 1024.f};
};