#ifndef UTILS_H
#define UTILS_H

#include <QPixmap>
#include <QString>

enum ELEMENT_OR_TILE {
    IS_TILE=1,
    IS_ELEMENT=2
};

enum DRAW_MODE {
    BRUSH_MODE,
    FILL_MODE,
    SELECT_MODE,
    ENEMY_MODE,
    PIPE_ENTER_MODE,
    PIPE_EXIT_MODE
};

enum ENEMY_TYPE {
    OIRAM_ENEMY=0xf0,
    RESWOB_ENEMY,
    SPIKE_ENEMY,
    FISH_ENEMY,
    GOOMBA_ENEMY,
    GREEN_KOOPA_ENEMY,
    RED_KOOPA_ENEMY,
    GREEN_KOOPA_FLY_ENEMY,
    RED_KOOPA_FLY_ENEMY,
    BONES_ENEMY,
    THWOMP_ENEMY,
    FIREBALL_ENEMY,
    CHOMPER_ENEMY,
    CHOMPER_FIRE_ENEMY,
    BOO_ENEMY
};

extern QPixmap *pixGoomba;
extern QPixmap *pixRedKoopa;
extern QPixmap *pixGreenKoopa;
extern QPixmap *pixRedKoopaFly;
extern QPixmap *pixGreenKoopaFly;
extern QPixmap *pixFish;
extern QPixmap *pixChomper;
extern QPixmap *pixFireChomper;
extern QPixmap *pixThwomp;
extern QPixmap *pixFireball;
extern QPixmap *pixBoo;
extern QPixmap *pixBones;
extern QPixmap *pixPipeUp;
extern QPixmap *pixPipeDown;
extern QPixmap *pixPipeRight;
extern QPixmap *pixPipeLeft;
extern QPixmap *pixOiram;
extern QPixmap *pixSpike;
extern QPixmap *pixReswob;
extern uint8_t tilemap_ids[16][16];
extern QPixmap *tileTileset;
extern int tmpID;

extern QColor backColor;

void initTilesetPixmap(QString path);
QPixmap idToPixmap(int id);
QRect idToRect(int id);
void recolorTilesetPixmap(QColor newColor);
void deletePixmaps(void);

#endif
