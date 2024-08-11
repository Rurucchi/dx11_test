//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
// rTexpacker v4.0 Atlas Descriptor Code exporter v2.0                          //
//                                                                              //
// more info and bugs-report:  github.com/raylibtech/rtools                     //
// feedback and support:       ray[at]raylibtech.com                            //
//                                                                              //
// Copyright (c) 2020-2024 raylib technologies (@raylibtech)                    //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

#define ATLAS_ATLAS_IMAGE_PATH      "atlas.png"
#define ATLAS_ATLAS_SPRITE_COUNT    3

// Atlas sprite properties
typedef struct rtpAtlasSprite {
    const char *nameId;
    int originX, originY;
    int positionX, positionY;
    int sourceWidth, sourceHeight;
    int padding;
    bool trimmed;
    int trimRecX, trimRecY, trimRecWidth, trimRecHeight;
} rtpAtlasSprite;

// Atlas sprites array
static rtpAtlasSprite rtpDescAtlas[3] = {
    { "hitcircle@2x", 0, 0, 520, 0, 240, 240, 0, false, 6, 5, 228, 230 },
    { "hitcircleoverlay@2x", 0, 0, 0, 0, 260, 260, 0, false, 0, 0, 260, 260 },
    { "approachcircle@2x", 0, 0, 260, 0, 260, 260, 0, false, 0, 0, 260, 260 },
};