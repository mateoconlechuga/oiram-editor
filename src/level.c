#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "level.h"

level_pack_t pack;
uint8_t curLevel = 0;

static uint16_t rgb888To1555(const uint8_t r8, const uint8_t g8, const uint8_t b8) {
    uint8_t r5 = round((int)r8 * 31.0 / 255.0);
    uint8_t g6 = round((int)g8 * 63.0 / 255.0);
    uint8_t b5 = round((int)b8 * 31.0 / 255.0);
    return ((g6 & 1) << 15) | (r5 << 10) | ((g6 >> 1) << 5) | b5;
}

static void rgb1555To888(const uint16_t c, int *r, int *g, int *b) {
    uint8_t r5 = (c >> 10) & 31;
    uint8_t b5 = (c) & 31;
    uint8_t g6 = ((c >> 4) & 62) | ((c >> 15) & 1);

    *r = (int)floor( r5 * 255.0 / 31.0 + 0.5);
    *g = (int)floor( g6 * 255.0 / 63.0 + 0.5);
    *b = (int)floor( b5 * 255.0 / 31.0 + 0.5);
}

void initPack(void) {
    pack.count = 0;
    pack.level = malloc(sizeof(level_t)*256);

    for(int i=0; i < 256; i++) {
        level_t *curr = &pack.level[i];
        curr->data = NULL;
        curr->r = 176;
        curr->g = 224;
        curr->b = 248;
        curr->oiramX = curr->oiramY = 255;
        for(int j=0; j<256; j++) {
            pipe_t *p = &curr->pipesDoors[j];
            p->enabled = false;
            p->enterX = p->enterY =
            p->exitX = p->exitY = 255;
            pack.level[curLevel].pipesDoorsCount = 0;
        }
    }
}

uint8_t findAvailablePipe(void) {
    uint8_t j;
    for(j=0; j<255; j++) {
        pipe_t *curr = &pack.level[curLevel].pipesDoors[j];
        if (!curr->enabled) {
            break;
        }
    }
    return j;
}

static uint16_t decode(uint8_t *in, uint8_t *out) {
    uint8_t c, i, cnt;
    uint16_t decompressed_size = 0;

    for(;;) {
        c = *in;
        in++;
        if (c == 255) return decompressed_size;
        if (c > 128) {
            cnt = c - 128;
            for (i = 0; i < cnt; i++) {
                *out = *in;
                out++; in++;
                decompressed_size++;
            }
        } else {
            cnt = c;
            c = *in;
            in++;
            for (i = 0; i < cnt; i++) {
                *out = c;
                out++;
                decompressed_size++;
            }
        }
    }
}

static uint16_t encode(uint8_t *in, uint8_t *out, size_t in_len) {
    uint8_t buf[256];
    uint8_t len = 0, repeat = 0, end = 0, c, i;
    uint16_t encoded_count = 0;

    while (!end) {
        end = !(in_len);
        c = *in;
        in++;
        in_len--;

        if (!end) {
            buf[len++] = c;
            if (len <= 1) continue;
        }

        if (repeat) {
            if (buf[len - 1] != buf[len - 2])
                repeat = 0;
            if (!repeat || len == 129 || end) {
                /* write out repeating bytes */
                *out = end ? len : len - 1;
                out++;
                *out = buf[0];
                out++;
                encoded_count += 2;
                buf[0] = buf[len - 1];
                len = 1;
            }
        } else {
            if (buf[len - 1] == buf[len - 2]) {
                repeat = 1;
                if (len > 2) {
                    *out = 128 + len - 2;
                    out++;
                    encoded_count++;
                    for (i = 0; i < len-2; i++) {
                        *out = buf[i];
                        out++;
                        encoded_count++;
                    }
                    buf[0] = buf[1] = buf[len - 1];
                    len = 2;
                }
                continue;
            }
            if (len == 128 || end) {
                *out = 128 + len;
                out++;
                encoded_count++;
                for (i = 0; i < len; i++) {
                    *out = buf[i];
                    out++;
                    encoded_count++;
                }
                len = 0;
                repeat = 0;
            }
        }
    }

    *out = 255;
    encoded_count++;
    return encoded_count;
}

uint8_t chkPackHasOiram(void) {
    for(uint8_t i=0; i<pack.count; i++) {
        if (pack.level[i].oiramX == 255 && pack.level[i].oiramY == 255) {
            return i+1;
        }
    }
    return 255;
}

bool loadFilePack(const char *filename, char **description, char **var, char **author) {
    FILE *in_file = fopen( filename, "rb" );
    uint8_t *input = NULL;
    uint16_t *level_offsets;
    size_t s;

    unsigned int i;
    unsigned int offset = 0x4C;
    unsigned int levels;
    unsigned int tOffset;
    unsigned int name_offset;
    unsigned int author_offset;

    if (!in_file) { goto err; }

    input = calloc(0x10100, 1);
    s = fread(input, 1, 0x10100, in_file);

    if (!s || input[0x4A] != 0xAB || input[0x4B] != 0xCD) {
        goto err;
    }

    for (i=0; i<256; i++) {
        level_t *curr = &pack.level[i];
        if (curr->data) {
            free(curr->data);
            curr->data = NULL;
        }
        for (unsigned int j=0; j<256; j++) {
            pipe_t *p = &curr->pipesDoors[j];
            p->enabled = false;
            p->enterX = p->enterY = p->exitX = p->exitY = 255;
        }
    }

    name_offset = offset;
    offset += strlen((const char*)&input[offset]) + 1;
    author_offset = offset;
    offset += strlen((const char*)&input[offset]) + 1;
    *description = strdup((const char*)&input[name_offset]);
    *author = strdup((const char*)&input[author_offset]);
    *var = strdup((const char*)&input[0x3c]);

    pack.count = input[offset++];
    level_offsets = (uint16_t*)&input[offset];
    levels = offset += ((pack.count-1)*2);

    // looking at the first level
    for(i=0; i<pack.count; i++) {
        level_t *curr = &pack.level[i];

        uint16_t color = input[offset] | (input[offset+1] << 8);
        rgb1555To888(color, &curr->r, &curr->g, &curr->b);

        offset += 2;
        curr->pipesDoorsCount = input[offset++];
        tOffset = offset;
        offset += curr->pipesDoorsCount * 6;
        curr->width = input[offset++];
        curr->height = input[offset++];

        for (unsigned int k=0; k<curr->pipesDoorsCount; k++) {
            pipe_t *pipeDoor = &curr->pipesDoors[k];
            unsigned int tExit, mExit;
            unsigned int tEnter, mEnter;
            tEnter = input[tOffset] | (input[tOffset+1] << 8) |  (input[tOffset+2] << 16);
            mEnter = tEnter & ~MASK_PIPE_DOOR;
            tOffset += 3;
            tExit = input[tOffset] | (input[tOffset+1] << 8) |  (input[tOffset+2] << 16);
            mExit = tExit & ~MASK_PIPE_DOOR;
            tOffset += 3;

            pipeDoor->enabled = true;
            pipeDoor->enterX = (mEnter % curr->width);
            pipeDoor->enterY = (mEnter / curr->width);
            pipeDoor->enterDir = tEnter & MASK_PIPE_DOOR;
            if (pipeDoor->enterDir & (MASK_PIPE_LEFT | MASK_PIPE_RIGHT)) { pipeDoor->enterY--; }
            pipeDoor->exitX = (mExit % curr->width);
            pipeDoor->exitY = (mExit / curr->width);
            pipeDoor->exitDir = tExit & MASK_PIPE_DOOR;
            if (pipeDoor->exitDir & (MASK_PIPE_LEFT | MASK_PIPE_RIGHT)) { pipeDoor->exitY--; }
        }
        curr->data = malloc(curr->width * curr->height);
        decode(&input[offset], curr->data);
        offset = levels + level_offsets[i];
    }

    fclose(in_file);
    free(input);
    return true;
err:
    if (in_file) { fclose( in_file ); }
    free(input);
    return false;
}

bool saveFilePack(const char *filename, const char *description, const char *varname, const char *author) {
    FILE *out_file = NULL;

    /* header for TI files */
    uint8_t header[]  = { 0x2A,0x2A,0x54,0x49,0x38,0x33,0x46,0x2A,0x1A,0x0A,'O','i','r','a','m',' ','L','e','v','e','l','P','a','c','k' };
    uint8_t len_high;
    uint8_t len_low;
    uint8_t *output = NULL;

    uint16_t *level_offsets;
    unsigned int levels;

    unsigned int i;
    unsigned int offset;
    int checksum;

    unsigned int name_length = strlen(varname);
    unsigned int description_length = strlen(description);
    unsigned int author_length = strlen(author);
    int data_length;
    int output_size;

    out_file = fopen( filename, "wb" );
    if ( !out_file ) { goto err; }

    output = calloc( 0x100000, 1 );

    for( i=0; i<sizeof header; ++i) {
        output[i] = header[i];
    }

    offset = 0x3C;
    for ( i=0; i<name_length; i++) {
        output[offset++] = varname[i];
    }

    offset = 0x4A;

    /**
     * Pack format
     * 0xabcd - header bytes               ( 2 bytes )
     * description + null terminator       ( n bytes <= 25 )
     * author + null terminator            ( n bytes <= 20 )
     * #levels                             ( 1 byte )
     * offsets to each compressed level    ( #levels*2 bytes )
     *  Level format
     *  color                              ( 2 bytes )
     *  #pipes                             ( 1 byte )
     *  pipe information                   ( #pipes*3 )
     *
     *  width                              ( 1 byte )
     *  height                             ( 1 byte )
     *  data -- compressed                 ( n bytes )
     */
    output[offset++] = 0xAB;
    output[offset++] = 0xCD;

    for ( i=0; i<description_length && i<26; i++) {
        output[offset++] = description[i];
    }

    output[offset++] = 0;
    for ( i=0; i<author_length && i<21; i++) {
        output[offset++] = author[i];
    }
    output[offset++] = 0;
    output[offset++] = pack.count;

    level_offsets = (uint16_t*)&output[offset];
    levels = offset += (pack.count-1)*2; // offsets

    for (i=0; i<pack.count; i++) {
        level_t *curr = &pack.level[i];
        size_t level_data_size = curr->height*curr->width;
        size_t prev_data_size;

        uint16_t color = rgb888To1555(curr->r, curr->g, curr->b);

        output[offset++] = color & 0xff;
        output[offset++] = color >> 8;
        output[offset++] = curr->pipesDoorsCount;

        if (curr->pipesDoorsCount) {
            for (int k=0; k<256; k++) {
                pipe_t *pipe = &curr->pipesDoors[k];
                if (pipe->enabled) {
                    unsigned int offset_mask = (pipe->enterX + (pipe->enterY * curr->width)) | pipe->enterDir;
                    if (pipe->enterDir & (MASK_PIPE_LEFT | MASK_PIPE_RIGHT)) { offset_mask += curr->width; }
                    output[offset++] = (offset_mask >>  0) & 0xff;
                    output[offset++] = (offset_mask >>  8) & 0xff;
                    output[offset++] = (offset_mask >> 16) & 0xff;
                    offset_mask = (pipe->exitX + (pipe->exitY * curr->width)) | pipe->exitDir;
                    if (pipe->exitDir & (MASK_PIPE_LEFT | MASK_PIPE_RIGHT)) { offset_mask += curr->width; }
                    output[offset++] = (offset_mask >>  0) & 0xff;
                    output[offset++] = (offset_mask >>  8) & 0xff;
                    output[offset++] = (offset_mask >> 16) & 0xff;
                }
            }
        }

        output[offset++] = curr->width;
        output[offset++] = curr->height;
        curr->compressed_size = encode(curr->data, &output[offset], level_data_size);
        offset += curr->compressed_size;

        if (i > 0) {
            level_offsets[i-1] = prev_data_size;
        }
        prev_data_size = offset - levels;
    }

    output[0x37] = 0x0D;      /* nessasary */
    output[0x3B] = 0x15;      /* appvar type */
    output[0x45] = 0x80;      /* archived */

    data_length = (offset - 0x37);
    len_high = (data_length>>8)&0xFF;
    len_low = (data_length&0xFF);
    output[0x35] = len_low;
    output[0x36] = len_high;

    data_length = (offset-0x4A);
    len_high = (data_length>>8)&0xFF;
    len_low = (data_length&0xFF);
    output[0x48] = len_low;
    output[0x49] = len_high;

    data_length += 2;
    len_high = (data_length>>8)&0xFF;
    len_low = (data_length&0xFF);
    output[0x39] = len_low;
    output[0x3A] = len_high;
    output[0x46] = len_low;
    output[0x47] = len_high;

    checksum = 0;
    for( i=0x37; i<offset; ++i) {
        checksum = (checksum + output[i])&0xFFFF;
    }

    output[offset++] = (checksum&0xFF);
    output[offset++] = (checksum>>8)&0xFF;

    output_size = data_length+name_length+7;

    if(output_size > 0xFFFF-30) { goto err;}

    fwrite( output, 1, offset, out_file );

    fclose( out_file );
    free( output );
    return true;
err:
    if (out_file) { fclose( out_file ); }
    free( output );
    return false;
}

void deletePack(void) {
    for(int i=0; i < 256; i++) {
        level_t *curr = &pack.level[i];
        if (curr->data) { free(curr->data); curr->data = NULL; }
    }
    free(pack.level);
}

void setLevel(uint8_t i, level_t *newLevel) {
    size_t size = newLevel->height * newLevel->width;
    level_t *curr = &pack.level[i];

    curr->height = newLevel->height;
    curr->width = newLevel->width;
    if (curr->data) { free(curr->data); curr->data = NULL; }
    curr->data = malloc(size);
    memcpy(curr->data, newLevel->data, size);
}

bool moveLevelUp(uint8_t i) {
    level_t temp;
    if (i == 0) { return false; }
    temp = pack.level[i];
    pack.level[i] = pack.level[i-1];
    pack.level[i-1] = temp;
    return true;
}

bool moveLevelDown(uint8_t i) {
    level_t temp;
    if (i > pack.count) { return false; }
    temp = pack.level[i];
    pack.level[i] = pack.level[i+1];
    pack.level[i+1] = temp;
    return true;
}

bool insertLevel(uint8_t i) {
    level_t *cur = &pack.level[i];
    if (pack.count == 250) { return false; }

    pack.count++;
    for(int j = pack.count; j > i; j--) {
        pack.level[j] = pack.level[j-1];
    }

    cur->width = 20;
    cur->height = 10;
    cur->data = malloc(20*10);
    cur->pipesDoorsCount = 0;
    memset(cur->data, 27, 20*10);
    return true;
}

void removeLevel(uint8_t i) {
    level_t *curr = &pack.level[i];
    uint8_t num_levels_less = pack.count--;

    if (curr->data) { free(curr->data); curr->data = NULL; }
    for(; i < num_levels_less; i++) {
        pack.level[i] = pack.level[i+1];
    }
}

level_t *getLevel(uint8_t i) {
    return &pack.level[i];
}
