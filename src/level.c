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

    for(int i = 0; i < 256; i++) {
        level_t *curr = &pack.level[i];
        curr->data = NULL;
        curr->r = 176;
        curr->g = 224;
        curr->b = 248;
        curr->oiramX = curr->oiramY = 255;
        curr->scroll = SCROLL_NONE;
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
    unsigned int j;
    for (j = 0; j < 256; j++) {
        pipe_t *curr = &pack.level[curLevel].pipesDoors[j];
        if (!curr->enabled) {
            break;
        }
    }
    return (uint8_t)j;
}

uint32_t computeHash(const uint8_t *buf, int len) {
    static const uint32_t crc32_table[] = {
      0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
      0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
      0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
      0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
      0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
      0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
      0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
      0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
      0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
      0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
      0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
      0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
      0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
      0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
      0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
      0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
      0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
      0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
      0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
      0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
      0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
      0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
      0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
      0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
      0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
      0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
      0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
      0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
      0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
      0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
      0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
      0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
      0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
      0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
      0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
      0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
      0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
      0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
      0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
      0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
      0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
      0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
      0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
      0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
      0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
      0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
      0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
      0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
      0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
      0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
      0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
      0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
      0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
      0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
      0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
      0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
      0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
      0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
      0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
      0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
      0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
      0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
      0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
      0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
    };
    uint32_t crc = 0xffffffff;
    while (len--) {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
        buf++;
    }
    return crc;
}

static uint16_t decode(uint8_t *in, uint8_t *out) {
    uint8_t c, i, cnt;
    uint16_t decompressed_size = 0;

    for (;;) {
        c = *in;
        in++;
        if (c > 128) {
            if (c == 255) {
                return decompressed_size;
            }
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
        if (!end) {
            c = *in;
        }
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
            if (!end && buf[len - 1] == buf[len - 2]) {
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

#define PACK_VERSION_MAGIC_MARKER 0xAB
#define PACK_VERSION_MAGIC_0 0xCD
#define PACK_VERSION_MAGIC_1 0xCE

bool getPackMetadata(const char *filename, uint32_t *data) {
    uint8_t tmp[90];
    FILE *fd;
    size_t s;
    uint8_t *c;

    if (data == NULL || filename == NULL) {
        return false;
    }

    fd = fopen(filename, "rb");
    if (!fd) {
        return false;
    }

    s = fread(tmp, sizeof tmp, 1, fd);
    if (!(s == 1 && tmp[0x4A] == PACK_VERSION_MAGIC_MARKER)) {
        fclose(fd);
        return false;
    }

    if (tmp[0x4B] == PACK_VERSION_MAGIC_0) {
        *data = 0;
        return true;
    }

    c = &tmp[0x4C];
    *data = (uint32_t)((*(c + 0) << 24) | (*(c + 1) << 16) | (*(c + 2) << 8) | (*(c + 3) << 0));
    fclose(fd);

    return true;
}

bool loadFilePack(const char *filename, char **description, char **var, char **author) {
    FILE *in_file = fopen(filename, "rb");
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

    if (!s || input[0x4A] != PACK_VERSION_MAGIC_MARKER) {
        goto err;
    }

    for (i = 0; i < 256; i++) {
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

    if (input[0x4B] == PACK_VERSION_MAGIC_1) {
        offset += sizeof(uint32_t);
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
        if (input[offset] == 255) {
            offset++;
            curr->scroll = input[offset];
            offset++;
        }
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

bool saveFilePack(const char *filename, const char *description, const char *varname, const char *author, uint32_t hash) {
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
    size_t prev_data_size;
    size_t level_data_size;
    uint16_t color;

    out_file = fopen( filename, "wb" );
    if (!out_file) { goto err; }

    output = calloc( 0x100000, 1 );

    for (i = 0; i < sizeof header; ++i) {
        output[i] = header[i];
    }

    offset = 0x3C;
    for (i = 0; i < name_length; i++) {
        output[offset++] = varname[i];
    }

    offset = 0x4A;

    /**
     * Pack format
     * header bytes                        ( 2 bytes )
     * password                            ( 4 bytes )
     * description + null terminator       ( n bytes <= 25 )
     * author + null terminator            ( n bytes <= 20 )
     * #levels                             ( 1 byte )
     * offsets to each compressed level    ( #levels*2 bytes )
     *  Level format
     *  color                              ( 2 bytes )
     *  indicator byte for scroll          ( 1 byte == 255 )
     *  scroll direction                   ( 1 byte <= 2 )
     *  #pipes                             ( 1 byte )
     *  pipe information                   ( #pipes*3 )
     *
     *  width                              ( 1 byte )
     *  height                             ( 1 byte )
     *  data -- compressed                 ( n bytes )
     */
    output[offset++] = PACK_VERSION_MAGIC_MARKER;
    output[offset++] = PACK_VERSION_MAGIC_1;
    output[offset++] = (hash & 0xff000000) >> 24;
    output[offset++] = (hash & 0x00ff0000) >> 16;
    output[offset++] = (hash & 0x0000ff00) >>  8;
    output[offset++] = (hash & 0x000000ff) >>  0;

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
        level_data_size = curr->height * curr->width;
        color = rgb888To1555(curr->r, curr->g, curr->b);

        // write color
        output[offset++] = color & 0xff;
        output[offset++] = color >> 8;

        // set scroll
        if (curr->scroll) {
            output[offset++] = 255;
            output[offset++] = curr->scroll;
        }

        // write # of pipes
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
    cur->scroll = SCROLL_NONE;
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
