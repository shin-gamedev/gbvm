#pragma bank 255

// Scene: Cave
// Sprites

#include "gbs_types.h"
#include "data/spritesheet_3.h"
#include "data/spritesheet_4.h"
#include "data/spritesheet_5.h"
#include "data/spritesheet_6.h"

BANKREF(scene_0_sprites)

const far_ptr_t scene_0_sprites[] = {
    TO_FAR_PTR_T(spritesheet_3),
    TO_FAR_PTR_T(spritesheet_4),
    TO_FAR_PTR_T(spritesheet_5),
    TO_FAR_PTR_T(spritesheet_6)
};
