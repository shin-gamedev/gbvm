#pragma bank 2

#include <string.h>
#include "vm.h"

// define addressmod for HOME
void ___vm_dummy_fn(void) NONBANKED __preserves_regs(a, b, c, d, e, h, l);
__addressmod ___vm_dummy_fn const HOME;

// here we define all VM instructions: their handlers and parameter lengths in bytes
// this array must be nonbanked as well as STEP_VM()
HOME const SCRIPT_CMD script_cmds[] = {
    {&vm_push,         2}, // 0x01
    {&vm_pop,          1}, // 0x02
    {&vm_call_rel,     1}, // 0x03
    {&vm_call,         2}, // 0x04
    {&vm_ret,          0}, // 0x05
    {&vm_loop_rel,     1}, // 0x06
    {&vm_loop,         2}, // 0x07
    {&vm_jump_rel,     1}, // 0x08
    {&vm_jump,         2}, // 0x09
    {&vm_call_far,     3}, // 0x0A
    {&vm_ret_far,      0}, // 0x0B
    {&vm_systime,      0}, // 0x0C
    {&vm_invoke,       4}, // 0x0D
    {&vm_beginthread,  3}, // 0x0E
    {&vm_ifcond,       6}, // 0x0F
    {&vm_debug,        2}, // 0x10
    {&vm_pushvalue,    1}, // 0x11
    {&vm_reserve,      1}, // 0x12
    {&vm_set,          2}, // 0x13
    {&vm_set_const,    3}, // 0x14
};


// contexts for executing scripts 
// ScriptRunnerInit(), ExecuteScript(), ScriptRunnerUpdate() manipulate these contexts
SCRIPT_CTX CTXS[SCRIPT_MAX_CONTEXTS];
SCRIPT_CTX * first_ctx, * free_ctxs;

// we need __banked functions here to have two extra words before arguments
// we will put VM stuff there
// plus we get an ability to call them from wherever we want in native code
// you can manipulate context (THIS) within VM functions
// if VM function has no parameters and does not manipulate context
// then you may declare it without params at all bacause caller clears stack - that is safe

// this is a call instruction, it pushes return address onto VM stack
void vm_call_rel(SCRIPT_CTX * THIS, INT8 ofs) __banked {
    // push current VM PC onto VM stack
    *(THIS->stack_ptr) = (UWORD)THIS->PC;
    THIS->stack_ptr++;
    // modify VM PC (goto PC + ofs)
    // pc is a pointer, you may point to any other script wherever you want
    // you may also pass absolute pointer instead of ofs, if you want
    THIS->PC += ofs;    
}
// call absolute instruction
void vm_call(SCRIPT_CTX * THIS, UBYTE * pc) __banked {
    *(THIS->stack_ptr) = (UWORD)THIS->PC;
    THIS->stack_ptr++;
    THIS->PC = pc;    
}
// return instruction returns to a point where call was invoked
void vm_ret(SCRIPT_CTX * THIS) __banked {
    // pop VM PC from VM stack
    THIS->stack_ptr--;
    THIS->PC = (const UBYTE *)*(THIS->stack_ptr);
}

// far call to another bank
void vm_call_far(SCRIPT_CTX * THIS, UBYTE bank, UBYTE * pc) __banked {
    *(THIS->stack_ptr) = (UWORD)THIS->PC;
    THIS->stack_ptr++;
    *(THIS->stack_ptr) = THIS->bank;
    THIS->stack_ptr++;
    THIS->PC = pc;
    THIS->bank = bank;
}
// ret from far call
void vm_ret_far(SCRIPT_CTX * THIS) __banked {
    THIS->stack_ptr--;
    THIS->bank = (UBYTE)(*(THIS->stack_ptr));
    THIS->stack_ptr--;
    THIS->PC = (const UBYTE *)*(THIS->stack_ptr);
}

// you can also invent calling convention and pass parameters to scripts on VM stack,
// make a library of scripts and so on
// pushes word onto VM stack
void vm_push(SCRIPT_CTX * THIS, UWORD value) __banked {
    *(THIS->stack_ptr) = value;
    THIS->stack_ptr++;
}
// cleans up to n words from stack and returns last one 
 UWORD vm_pop(SCRIPT_CTX * THIS, UBYTE n) __banked {
    THIS->stack_ptr -= n;
    return *(THIS->stack_ptr);
}

// do..while loop, callee cleanups stack
void vm_loop_rel(SCRIPT_CTX * THIS, INT8 ofs) __banked {
    UWORD * counter = THIS->stack_ptr - 1;
    if (*counter) THIS->PC += ofs, (*counter)--; else vm_pop(THIS, 1);
}
// loop absolute, callee cleanups stack
void vm_loop(SCRIPT_CTX * THIS, UINT8 * pc) __banked {
    UWORD * counter = THIS->stack_ptr - 1;
    if (*counter) THIS->PC = pc, (*counter)--; else vm_pop(THIS, 1);
}

// jump relative
void vm_jump_rel(SCRIPT_CTX * THIS, INT8 ofs) __banked {
    THIS->PC += ofs;    
}
// jump absolute
void vm_jump(SCRIPT_CTX * THIS, UBYTE * pc) __banked {
    THIS->PC = pc;    
}

// push systime on VM stack 
void vm_systime(SCRIPT_CTX * THIS) __banked {
    *(THIS->stack_ptr) = sys_time;
    THIS->stack_ptr++;
} 

UBYTE wait_frames(void * THIS, UBYTE start, UBYTE nparams, UWORD * stack_frame) __banked {
    THIS; nparams; // suppress warnings
    // we allocate one local variable (just write ahead of VM stack pointer, we have no interrupts, our local variables won't get spoiled)
    if (start) stack_frame[1] = sys_time;
    // check wait condition
    return ((sys_time - stack_frame[1]) > stack_frame[0]);
}
// calls C handler until it returns true; callee cleanups stack
void vm_invoke(SCRIPT_CTX * THIS, UBYTE bank, UBYTE * fn, UBYTE nparams) __banked {
    FAR_PTR newptr = to_far_ptr(fn, bank);
    UWORD * stack_frame = THIS->stack_ptr - nparams;

    // update function pointer
    if (THIS->update_fn != newptr) {
        THIS->update_fn = newptr;
        // call here with init == true
        FAR_CALL(newptr, SCRIPT_UPDATE_FN, THIS, 1, nparams, stack_frame);
    }
    if (FAR_CALL(newptr, SCRIPT_UPDATE_FN, THIS, 0, nparams, stack_frame)) {
        // pop param words from VM stack (callee clears VM stack rule) 
        THIS->stack_ptr -= nparams;
        // cleanup update function pointer
        THIS->update_fn = 0;
        return;
    }
    // call handler again, wait condition is not met
    THIS->PC -= (INSTRUCTION_SIZE + sizeof(bank) + sizeof(fn) + sizeof(nparams));
} 

// runs script in a new thread
void vm_beginthread(SCRIPT_CTX * THIS, UBYTE bank, UBYTE * pc) __banked {
    THIS; // suppress warnings
    ExecuteScript(bank, pc);
}
// if condition; compares two arguments on VM stack
// idxA, idxB point to arguments to compare
// negative indexes are parameters on the top of VM stack, positive - absolute indexes in stack[] array
void vm_ifcond(SCRIPT_CTX * THIS, UBYTE condition, INT8 idxA, INT8 idxB, UBYTE * pc, UBYTE n) __banked {
    INT16 A, B;
    if (idxA < 0) A = *(THIS->stack_ptr + idxA); else A = THIS->stack[idxA];
    if (idxB < 0) B = *(THIS->stack_ptr + idxB); else B = THIS->stack[idxB];
    UBYTE res = 0;
    switch (condition) {
        case 0: res = (A == B); break;
        case 1: res = (A <  B); break;
        case 2: res = (A >  B); break;
        case 3: res = (A <= B); break;
        case 4: res = (A >= B); break;
    }
    if (res) THIS->PC = pc;
    THIS->stack_ptr -= n;
}
// prints debug string
void vm_debug(SCRIPT_CTX * THIS, char * str) __banked {
    THIS; // suppress warnings
    puts(str);
}
// pushes value from VM stack onto VM stack
// if idx >= 0 then idx is absolute, else idx is relative to VM stack pointer
void vm_pushvalue(SCRIPT_CTX * THIS, INT8 idx) __banked {
    if (idx < 0) *(THIS->stack_ptr) = *(THIS->stack_ptr + idx); else *(THIS->stack_ptr) = THIS->stack[idx];
    THIS->stack_ptr++;
}
// manipulates VM stack pointer
// allows to reserve (or free if negative) ofs words on stack. if you do that at the beginning of script
// then it allows to use beginning of THIS->stack[] array as script global variables
void vm_reserve(SCRIPT_CTX * THIS, INT8 ofs) __banked {
    THIS->stack_ptr += ofs;
}
// sets value on stack indexed by idxA to value on stack indexed by idxB 
void vm_set(SCRIPT_CTX * THIS, INT8 idxA, INT8 idxB) __banked {
    INT16 * A, * B;
    if (idxA < 0) A = THIS->stack_ptr + idxA; else A = &(THIS->stack[idxA]);
    if (idxB < 0) B = THIS->stack_ptr + idxB; else B = &(THIS->stack[idxB]);
    *A = *B;
}
// sets value on stack indexed by idx to value
void vm_set_const(SCRIPT_CTX * THIS, INT8 idx, UWORD value) __banked {
    UWORD * A;
    if (idx < 0) A = THIS->stack_ptr + idx; else A = &(THIS->stack[idx]);
    *A = value;
}

// return zero if script end
// bank with VM code must be active
UBYTE STEP_VM(SCRIPT_CTX * CTX) __naked __nonbanked __preserves_regs(b, c) {
    CTX;
__asm
        lda hl, 2(sp)
        ld a, (hl+)
        ld h, (hl)
        
        add #2
        ld l, a
        adc h
        sub l
        ld h, a

        ld a, (hl-)
        ld e, a
        ld a, (hl-)
        ld l, (hl)
        ld h, a

        ldh a, (__current_bank)
        push af

        ld a, e
        ldh (__current_bank), a
        ld (0x2000), a          ; switch bank with vm code
        
        ld a, (HL+)             ; load current command and return if terminator
        ld e, a
        or a
        jr z, 3$

        push bc                 ; store bc
        push hl

        add a
        add e                   ; a = a * sizeof(SCRIPT_CMD)

        ld hl, #_script_cmds
        add l
        ld l, a
        adc h
        sub l
        ld h, a                 ; hl = &script_cmds[command+1]
        dec hl                  ; hl = &script_cmds[command].args_len

        ld a, (hl-)
        ld e, a                 ; e = args_len
        ld a, (hl-)
        ld l, (hl)
        ld h, a                 ; hl = fn

        pop bc                  ; bc points to the next VM instruction or a first byte of the args
        ld a, e
        or a
        ld d, a
        jr z, 1$                ; no args?
2$:                             ; copy args onto stack
        ld a, (bc)
        inc bc
        push af
        inc sp
        dec d
        jr nz, 2$
1$:                             ; bc points to the next VM instruction
        push hl                 ; save function pointer

        lda hl, 8(sp)
        add hl, de              ; add correction
        ld a, (hl+)
        ld h, (hl)
        ld l, a
        ld (hl), c
        ld c, l
        ld a, h
        inc hl
        ld (hl), b              ; PC = PC + sizeof(instruction) + args_len
        ld b, a                 ; bc = &PC

        pop hl                  ; restore function pointer
        push bc                 ; pushing THIS

        push de                 ; not used
        push de                 ; d: fn_bank, e: args_len

        ld a, #b_vm_call        ; a = script_bank (all script functions in one bank: take any complimantary symbol)
        ldh (__current_bank), a
        ld (0x2000), a          ; switch bank with functions

        rst 0x20                ; call hl

        pop hl                  ; h: _current_bank, l: args_len

        ld  h, #0
        ld  a, #4
        add l
        ld l, a
        add hl, sp              ; deallocate dummy word, this and args_len bytes from the stack
        ld sp, hl

        pop bc                  ; restore bc

        ld e, #1                ; command executed
3$:     
        pop af
        ldh (__current_bank), a
        ld (0x2000), a          ; restore bank

        ret
__endasm;
}

void ___vm_dummy_fn(void) __nonbanked __preserves_regs(a, b, c, d, e, h, l) __naked { __asm__("ret"); }

void ScriptRunnerInit() __banked {
    free_ctxs = CTXS, first_ctx = 0;
    memset(CTXS, 0, sizeof(CTXS));
    SCRIPT_CTX * tmp = CTXS;
    for (UBYTE i = 0; i < (SCRIPT_MAX_CONTEXTS - 1); i++) {
        tmp->next = tmp + 1;
        tmp++;
    }
}

UBYTE ExecuteScript(UBYTE bank, UBYTE * pc) __banked {
    if (free_ctxs) {
        SCRIPT_CTX * tmp = free_ctxs;
        // remove context from free list
        free_ctxs = free_ctxs->next;
        // initialize context
        tmp->PC = pc, tmp->bank = bank, tmp->stack_ptr = tmp->stack;
        // add context to active list
        tmp->next = first_ctx, first_ctx = tmp;
        return 1;
    }
    return 0;
}

UBYTE ScriptRunnerUpdate() __nonbanked {
    static SCRIPT_CTX * old_ctx, * ctx;
    old_ctx = 0, ctx = first_ctx; 
    while (ctx) {
        if (!STEP_VM(ctx)) {
            // script is finished, remove from linked list
            if (old_ctx) old_ctx->next = ctx->next; else first_ctx = ctx->next;
            // add terminated context to free contexts list
            ctx->next = free_ctxs, free_ctxs = ctx;
            // next context
            if (old_ctx) ctx = old_ctx->next; else ctx = first_ctx;
        } else old_ctx = ctx, ctx = ctx->next;
    }
    // return true if not all threads are finished
    return (first_ctx != 0); 
}
