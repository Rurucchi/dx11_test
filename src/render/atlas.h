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
#define ATLAS_ATLAS_SPRITE_COUNT    14

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
static rtpAtlasSprite rtpDescAtlas[14] = {
    { "hitcircle@2x", 0, 0, 520, 0, 240, 240, 0, false, 6, 5, 228, 230 },
    { "hitcircleoverlay@2x", 0, 0, 0, 0, 260, 260, 0, false, 0, 0, 260, 260 },
    { "default-0@2x", 0, 0, 609, 260, 87, 120, 0, false, 0, 3, 87, 116 },
    { "default-1@2x", 0, 0, 760, 0, 87, 120, 0, false, 12, 5, 64, 112 },
    { "default-5@2x", 0, 0, 847, 0, 87, 120, 0, false, 0, 2, 87, 117 },
    { "default-6@2x", 0, 0, 934, 0, 87, 120, 0, false, 0, 2, 87, 117 },
    { "default-7@2x", 0, 0, 0, 260, 87, 120, 0, false, 0, 3, 87, 116 },
    { "default-8@2x", 0, 0, 87, 260, 87, 120, 0, false, 0, 0, 87, 119 },
    { "default-9@2x", 0, 0, 174, 260, 87, 120, 0, false, 0, 2, 87, 117 },
    { "default-1@2x", 0, 0, 261, 260, 87, 120, 0, false, 12, 5, 64, 112 },
    { "default-2@2x", 0, 0, 348, 260, 87, 120, 0, false, 0, 2, 87, 117 },
    { "default-3@2x", 0, 0, 435, 260, 87, 120, 0, false, 0, 2, 87, 117 },
    { "default-4@2x", 0, 0, 522, 260, 87, 120, 0, false, 0, 3, 87, 115 },
    { "approachcircle@2x", 0, 0, 260, 0, 260, 260, 0, false, 0, 0, 260, 260 },
};
