#pragma bank 255
// SpriteSheet: signpost
  
#include "gbs_types.h"
#include "data/tileset_4.h"

const void __at(255) __bank_spritesheet_2;

const metasprite_t spritesheet_2_metasprite_0[]  = {
    { 0, 8, 0, 0 }, { 0, -8, 2, 0 },
    {metasprite_end}
};

const metasprite_t * const spritesheet_2_metasprites[] = {
    spritesheet_2_metasprite_0
};

const struct spritesheet_t spritesheet_2 = {
    .n_metasprites = 1,
    .metasprites = spritesheet_2_metasprites,
    .animations = {
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        },
        {
            .start = 0,
            .end = 0
        }
    },
    .tileset = TO_FAR_PTR_T(tileset_4),
    .cgb_tileset = { NULL, NULL }
};
