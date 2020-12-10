#pragma bank 2

#include "vm.h"

#include "Actor.h"
#include "GameTime.h"

typedef struct act_move_to_t {
    UBYTE ID;
    UBYTE _pad0; 
    INT16 X, Y; 
    UBYTE H_FIRST;
    UBYTE _pad1; 
    UBYTE COLL;
} act_move_to_t;

void vm_actor_move_to(SCRIPT_CTX * THIS, INT16 idx) __banked {
    actor_t *actor;
    BYTE new_dir_x = 0;
    BYTE new_dir_y = 0;

    act_move_to_t * params = VM_REF_TO_PTR(idx);

    // indicate waitable state of context
    THIS->waitable = 1;

    actor = actors + params->ID;

    // if (start && stack_frame[4]) {
        // Check for collisions and modify
        // stack_frame[1] (x) and stack_frame[2] (y)
        // if obstacle is in the way
    // }

    // Actor reached destination
    if ((actor->x == params->X) && (actor->y == params->Y)) {
        actor_set_anim(actor, FALSE);
        return;
    }

    // Actor not at horizontal destination
    if ((actor->x != params->X) &&
        (!(params->H_FIRST) || (actor->y == params->Y))) {
        if (actor->x < params->X) {
            new_dir_x = 1;
        } else if (actor->x > params->X) {
            new_dir_x = -1;
        }
    } else {
        // Actor not at vertical destination
        // actor->dir.x = 0;
        if (actor->y < params->Y) {
            new_dir_y = 1;
        } else if (actor->y > params->Y) {
            new_dir_y = -1;
        }
    }

    // If changed direction, trigger actor rerender
    if ((actor->dir_x != new_dir_x) ||
        (actor->dir_y != new_dir_y)) {
        actor_set_dir(actor, new_dir_x, new_dir_y);
    }

    actor_set_anim(actor, TRUE);

    // Move actor
    if (actor->move_speed == 0) {
        // Half speed only move every other frame
        if (IS_FRAME_2) {
            actor->x += (WORD)new_dir_x;
            actor->y += (WORD)new_dir_y;
        }
    } else {
        actor->x += (WORD)(new_dir_x * actor->move_speed);
        actor->y += (WORD)(new_dir_y * actor->move_speed);
    }

    THIS->PC -= (INSTRUCTION_SIZE + sizeof(idx));
    return;
}

UBYTE vm_actor_activate(void * THIS, UBYTE start, UWORD * stack_frame) __banked {    
    THIS, start;
    actor_t *actor;
    actor = &actors[stack_frame[0]];
    activate_actor(actor);
    return TRUE;
}

UBYTE vm_actor_set_dir(void * THIS, UBYTE start, UWORD * stack_frame) __banked {
    THIS, start;
    actor_t *actor;
    actor = &actors[stack_frame[0]];
    actor_set_dir(actor, stack_frame[1], stack_frame[2]);
    return TRUE;
}
