#ifndef LEVEL_H
#define LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define TILE_WIDTH       (16)
#define TILE_HEIGHT      (16)
#define MASK_PIPE_DOWN   (0)
#define MASK_PIPE_UP     (1<<23)
#define MASK_PIPE_LEFT   (1<<22)
#define MASK_PIPE_RIGHT  (1<<21)
#define MASK_DOOR_E     (1<<20)
#define MASK_DOOR_X    (1<<19)
#define MASK_PIPE_DOOR   (MASK_PIPE_RIGHT | MASK_PIPE_LEFT | MASK_PIPE_UP | MASK_DOOR_E | MASK_DOOR_X)

// doors are treated as pipes
typedef struct {
    bool enabled;
    uint8_t enterX, enterY;
    uint8_t exitX, exitY;
    int enterDir, exitDir;
} pipe_t;

typedef struct {
    uint8_t width, height;
    uint8_t *data;
    pipe_t pipesDoors[256];
    uint8_t pipesDoorsCount;
    int r,g,b;
    unsigned int compressed_size;
    uint8_t oiramX, oiramY;
    uint8_t scroll;
} level_t;

typedef struct {
    level_t *level;
    unsigned int count;
} level_pack_t;

enum autoscroll {
    SCROLL_NONE=0,
    SCROLL_RIGHT,
    SCROLL_LEFT,
    SCROLL_DOWN,
    SCROLL_UP
};

extern level_pack_t pack;
extern uint8_t curLevel;

void deletePack(void);
uint32_t computeHash(const uint8_t *buf, int len);

bool getPackMetadata(const char *filename, uint32_t *data);
bool loadFilePack(const char *filename, char **description, char **var, char **author);
bool saveFilePack(const char *filename, const char *description, const char *varname, const char *author, uint32_t hash);
void setLevel(uint8_t i, level_t *level);
level_t *getLevel(uint8_t i);
void removeLevel(uint8_t i);
bool insertLevel(uint8_t i);
void initPack(void);

bool moveLevelUp(uint8_t i);
bool moveLevelDown(uint8_t i);
uint8_t chkPackHasOiram(void);

/* Pipe stuffs */
uint8_t findAvailablePipe(void);

#ifdef __cplusplus
}
#endif

#endif
