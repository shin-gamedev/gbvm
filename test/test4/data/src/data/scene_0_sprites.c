#pragma bank 255

// Scene: Outside
// Sprites

#include "gbs_types.h"
#include "data/spritesheet_1.h"

const void __at(255) __bank_scene_0_sprites;

const far_ptr_t scene_0_sprites[] = {
    TO_FAR_PTR_T(spritesheet_1)
};
