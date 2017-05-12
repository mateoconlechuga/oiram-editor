#include <QPixmap>
#include <QImage>
#include <QString>
#include <QBitmap>
#include <QPainter>

#include "level.h"
#include "utils.h"

uint8_t tilemap_ids[16][16];

uint8_t tilemap_id_loc[2*256] = {
    15, 15,   // 0
    15, 15,   // 1
    15, 15,   // 2
    15, 15,   // 3
    2, 0,     // 4
    15, 15,   // 5
    15, 15,   // 6
    15, 15,   // 7
    3, 0,     // 8
    15, 15,   // 9
    15, 15,   // 10
    1, 0,     // 11
    4, 0,     // 12
    15, 11,   // 13
    0, 1,     // 14
    1, 1,     // 15
    2, 1,     // 16
    3, 1,     // 17
    4, 1,     // 18
    5, 1,     // 19
    6, 1,     // 20
    7, 1,     // 21
    11, 3,    // 22
    12, 3,    // 23
    13, 3,    // 24
    14, 6,    // 25
    11, 6,    // 26
    0, 0,     // 27
    0, 2,     // 28
    1, 2,     // 29
    2, 2,     // 30
    3, 2,     // 31
    4, 2,     // 32
    5, 2,     // 33
    6, 2,     // 34
    7, 2,     // 35
    11, 4,    // 36
    12, 4,    // 37
    13, 4,    // 38
    15, 5,    // 39
    14, 5,    // 40
    5, 0,     // 41
    11, 0,    // 42
    12, 0,    // 43
    13, 0,    // 44
    11, 7,    // 45
    12, 7,    // 46
    13, 7,    // 47
    7, 0,     // 48
    8, 0,     // 49
    9, 0,     // 50
    8, 1,     // 51
    9, 1,     // 52
    10, 1,    // 53
    7, 3,     // 54
    13, 6,    // 55 lava
    11, 1,    // 56
    12, 1,    // 57
    13, 1,    // 58
    14, 7,    // 59
    15, 7,    // 60
    12, 7,    // 61
    10, 0,    // 62
    15, 1,    // 63
    14, 1,    // 64
    8, 2,     // 65
    9, 2,     // 66
    10, 2,    // 67
    15, 6,    // 68
    15, 0,    // 69
    6, 11,    // 70 bullet
    0, 3,     // 71
    1, 3,     // 72
    2, 3,     // 73
    3, 3,     // 74
    4, 3,     // 75
    5, 3,     // 76
    8, 3,     // 77
    9, 3,     // 78
    10, 3,    // 79
    11, 2,    // 80
    12, 2,    // 81
    13, 2,    // 82
    14, 7,    // 83 cannon up
    6, 12,    // 84 bullet 1
    0, 4,     // 85
    1, 4,     // 86
    2, 4,     // 87
    3, 4,     // 88
    4, 4,     // 89
    5, 4,     // 90
    8, 4,     // 91
    9, 4,     // 92
    10, 4,    // 93
    9, 6,     // 94
    15, 15,   // 95
    15, 15,   // 96
    15, 7,    // 97 cannon down
    6, 13,    // 98 bullet 2
    0, 5,     // 99
    1, 5,     // 100
    2, 5,     // 101
    3, 5,     // 102
    4, 5,     // 103
    5, 5,     // 104
    8, 5,     // 105
    9, 5,     // 106
    10, 5,    // 107
    14, 4,    // 108
    15, 15,   // 109
    15, 15,   // 110
    14, 0,    // 111
    14, 2,    // 112
    15, 2,    // 113
    14, 3,    // 114
    15, 3,    // 115
    2, 6,     // 116
    3, 6,     // 117
    12, 5,    // 118
    15, 15,   // 119
    15, 15,   // 120
    15, 15,   // 121
    13, 5,    // 122
    15, 15,   // 123
    15, 15,   // 124
    15, 15,   // 125
    11, 5,    // 126
    15, 15,   // 127
    15, 15,   // 128
    15, 15,   // 129
    6, 6,     // 130
    7, 6,     // 131
    12, 6,    // 132
    15, 15,   // 133
    15, 15,   // 134
    15, 15,   // 135
    8, 7,     // 136
    9, 7,     // 137
    10, 7,
    15, 4,
    12, 8,
    13, 8,
    14, 8,
    15, 8,
    2, 7,
    3, 7,
    10, 6,
    15, 15,
    15, 15,
    15, 15,
    6, 0,
    15, 15,
    15, 15,
    15, 15,
    12, 9,
    13, 9,
    14, 9,
    15, 9,
    6, 7,
    7, 7,
    6, 4,
    7, 4,
    0, 11,
    1, 11,
    2, 11,
    3, 11,
    4, 11,
    5, 11,
    12, 10,
    13, 10,
    14, 10,
    15, 10,
    4, 6,
    5, 6,
    6, 5,
    7, 5,
    8, 6,
    1, 12,
    6, 3,
    3, 12,
    4, 12,
    5, 12,
    0, 8,
    1, 8,
    2, 8,
    3, 8,
    4, 8,
    5, 8,
    6, 8,
    7, 8,
    8, 8,
    9, 8,
    10, 8,
    11, 8,
    0, 6,
    1, 6,
    0, 9,
    1, 9,
    2, 9,
    3, 9,
    4, 9,
    5, 9,
    6, 9,
    7, 9,
    8, 9,
    9, 9,
    10, 9,
    11, 9,
    0, 7,
    1, 7,
    0, 10,
    1, 10,
    2, 10,
    3, 10,
    4, 10,
    5, 10,
    6, 10,
    7, 10,
    8, 10,
    9, 10,
    10, 10,
    11, 10,
    4, 7,
    5, 7,
};

QPixmap *tileTileset;
QPixmap *pixGoomba;
QPixmap *pixRedKoopa;
QPixmap *pixGreenKoopa;
QPixmap *pixRedKoopaFly;
QPixmap *pixGreenKoopaFly;
QPixmap *pixFish;
QPixmap *pixChomper;
QPixmap *pixFireChomper;
QPixmap *pixThwomp;
QPixmap *pixFireball;
QPixmap *pixBoo;
QPixmap *pixBones;
QPixmap *pixPipeUp;
QPixmap *pixPipeDown;
QPixmap *pixPipeRight;
QPixmap *pixPipeLeft;
QPixmap *pixDoorEnter;
QPixmap *pixDoorExit;
QPixmap *pixOiram;
QPixmap *pixSpike;
QPixmap *pixReswob;
QString tilePath;
QColor backColor;

void recolorTilesetPixmap(QColor newColor) {
    QPixmap pix(tilePath);

    QBitmap mask(pix.createMaskFromColor(QColor(176, 224, 248), Qt::MaskOutColor));

    QPainter p(&pix);
    p.setPen(backColor = newColor);
    p.drawPixmap(pix.rect(), mask, mask.rect());
    delete tileTileset;
    tileTileset = new QPixmap(pix.copy(pix.rect()));
}

void deletePixmaps(void) {
    delete pixGoomba;
    delete pixRedKoopa;
    delete pixGreenKoopa;
    delete pixRedKoopaFly;
    delete pixGreenKoopaFly;
    delete pixFish;
    delete pixChomper;
    delete pixFireChomper;
    delete pixThwomp;
    delete pixFireball;
    delete pixBoo;
    delete pixBones;
    delete tileTileset;
    delete pixPipeUp;
    delete pixPipeDown;
    delete pixPipeRight;
    delete pixPipeLeft;
}

void initTilesetPixmap(QString path) {
    pixGoomba = new QPixmap(":/resources/enemies/goomba.png");
    pixRedKoopa = new QPixmap(":/resources/enemies/koopa_red.png");
    pixGreenKoopa = new QPixmap(":/resources/enemies/koopa_green.png");
    pixRedKoopaFly = new QPixmap(":/resources/enemies/koopa_red_fly.png");
    pixGreenKoopaFly = new QPixmap(":/resources/enemies/koopa_green_fly.png");
    pixFish = new QPixmap(":/resources/enemies/fish.png");
    pixChomper = new QPixmap(":/resources/enemies/chomper.png");
    pixFireChomper = new QPixmap(":/resources/enemies/chomper_fire.png");
    pixThwomp = new QPixmap(":/resources/enemies/thwomp.png");
    pixFireball = new QPixmap(":/resources/enemies/flame_fire.png");
    pixBoo = new QPixmap(":/resources/enemies/boo.png");
    pixBones = new QPixmap(":/resources/enemies/koopa_bones.png");
    pixReswob = new QPixmap(":/resources/enemies/bowser.png");
    pixSpike = new QPixmap(":/resources/enemies/spike.png");
    pixPipeUp = new QPixmap(":/resources/tiles/pipe_up.png");
    pixPipeDown = new QPixmap(":/resources/tiles/pipe_down.png");
    pixPipeLeft = new QPixmap(":/resources/tiles/pipe_left.png");
    pixPipeRight = new QPixmap(":/resources/tiles/pipe_right.png");
    pixDoorEnter = new QPixmap(":/resources/tiles/door_in.png");
    pixDoorExit = new QPixmap(":/resources/tiles/door_out.png");
    pixOiram = new QPixmap(":/resources/tiles/oiram.png");

    // ice coin block
    tilemap_id_loc[152*2] = 10;
    tilemap_id_loc[152*2+1] = 12;

    tilemap_id_loc[225*2] = 0;
    tilemap_id_loc[225*2+1] = 13;
    tilemap_id_loc[226*2] = 1;
    tilemap_id_loc[226*2+1] = 13;
    tilemap_id_loc[227*2] = 2;
    tilemap_id_loc[227*2+1] = 13;
    tilemap_id_loc[228*2] = 3;
    tilemap_id_loc[228*2+1] = 13;
    tilemap_id_loc[229*2] = 4;
    tilemap_id_loc[229*2+1] = 13;
    tilemap_id_loc[230*2] = 5;
    tilemap_id_loc[230*2+1] = 13;
    tilemap_id_loc[237*2] = 10;
    tilemap_id_loc[237*2+1] = 11;

    // snow tiles
    tilemap_id_loc[127*2] = 11;
    tilemap_id_loc[127*2+1] = 11;
    tilemap_id_loc[128*2] = 12;
    tilemap_id_loc[128*2+1] = 11;
    tilemap_id_loc[129*2] = 13;
    tilemap_id_loc[129+2+1] = 11;
    tilemap_id_loc[119*2] = 11;
    tilemap_id_loc[119*2+1] = 12;
    tilemap_id_loc[120*2] = 12;
    tilemap_id_loc[120*2+1] = 12;
    tilemap_id_loc[121*2] = 13;
    tilemap_id_loc[121+2+1] = 12;

    // end pipe
    tilemap_id_loc[59*2] = 14;
    tilemap_id_loc[59*2+1] = 11;
    tilemap_id_loc[60*2] = 15;
    tilemap_id_loc[60*2+1] = 11;
    tilemap_id_loc[109*2] = 14;
    tilemap_id_loc[109*2+1] = 12;
    tilemap_id_loc[110*2] = 15;
    tilemap_id_loc[110*2+1] = 12;

    // blue coins
    tilemap_id_loc[235*2] = 8;
    tilemap_id_loc[235*2+1] = 11;

    // door tiles
    tilemap_id_loc[231*2] = 7;
    tilemap_id_loc[231*2+1] = 12;
    tilemap_id_loc[232*2] = 7;
    tilemap_id_loc[232*2+1] = 11;

    // blue blocks
    tilemap_id_loc[238*2] = 9;
    tilemap_id_loc[238*2+1] = 11;

    // ice block
    tilemap_id_loc[239*2] = 9;
    tilemap_id_loc[239*2+1] = 12;

    tileTileset = new QPixmap(tilePath = path);
    for(unsigned int i=0; i<sizeof tilemap_id_loc; i+=2) {
        uint8_t xpos = tilemap_id_loc[i];
        uint8_t ypos = tilemap_id_loc[i+1];
        tilemap_ids[xpos][ypos] = i/2;
    }

    tilemap_ids[0][0] = 27;
    tilemap_ids[0][12] = 13;
    tilemap_ids[2][12] = 13;
    tilemap_ids[8][12] = 13;
    for (unsigned int i=7; i<16; i++) {
        tilemap_ids[i][13] = 13;
    }
}

QRect idToRect(int id) {
    QRect rect;
    switch(id) {
        case 0xf3: rect.adjust(0,0,16,16); break;
        case 0xf4: rect.adjust(0,0,16,16); break;
        case 0xf5: rect.adjust(0,5,16,27); break;
        case 0xf6: rect.adjust(0,5,16,27); break;
        case 0xf7: rect.adjust(0,5,16,27); break;
        case 0xf8: rect.adjust(0,5,16,27); break;
        case 0xf9: rect.adjust(0,5,16,27); break;
        case 0xfa: rect.adjust(4,0,24,32); break;
        case 0xfb: rect.adjust(0,0,16,16); break;
        case 0xfc: rect.adjust(8,0,32,16); break;
        case 0xfd: rect.adjust(8,0,32,16); break;
        case 0xfe: rect.adjust(0,0,16,16); break;
        default:
            rect.adjust(0,0,16,16);
            break;
    }
    return rect;
}

QPixmap idToPixmap(int id) {
    if (id == 243) {
        id = 26;
    }
    if (id >= 0xf0) {
        if (id == FIREBALL_ENEMY) {
            id = 122;
        } else {
            id = 27;
        }
    }
    return tileTileset->copy(tilemap_id_loc[id*2]*TILE_WIDTH, tilemap_id_loc[id*2+1]*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
}
