.module script_s7a15_interact

.include "vm.i"
.include "data/game_globals.i"

.area _CODE_255

.LOCAL_TMP0_PARAM0_VALUE = -1
.LOCAL_TMP1_PARAM0_VALUE = -1

___bank_script_s7a15_interact = 255
.globl ___bank_script_s7a15_interact

_script_s7a15_interact::
        VM_RESERVE              1

        ; If Parameter 0 Equals 0
        VM_GET_TLOCAL           .LOCAL_TMP0_PARAM0_VALUE, 0
        VM_IF_CONST .EQ         .LOCAL_TMP0_PARAM0_VALUE, 0, 1$, 0
        VM_JUMP                 2$
1$:
        ; Call Script: Enemy Ship Destroy
        VM_PUSH_CONST           16 ; Actor SCRIPT_ARG_0_ACTOR
        VM_CALL_FAR             ___bank_script_enemy_ship_destroy, _script_enemy_ship_destroy

        ; Stop Script
        VM_STOP
2$:

        ; If Parameter 0 Equals 2
        VM_GET_TLOCAL           .LOCAL_TMP1_PARAM0_VALUE, 0
        VM_IF_CONST .EQ         .LOCAL_TMP1_PARAM0_VALUE, 2, 3$, 0
        VM_JUMP                 4$
3$:
        ; Call Script: Enemy Ship Destroy
        VM_PUSH_CONST           16 ; Actor SCRIPT_ARG_0_ACTOR
        VM_CALL_FAR             ___bank_script_enemy_ship_destroy, _script_enemy_ship_destroy

        ; Stop Script
        VM_STOP
4$:

        ; Stop Script
        VM_STOP
