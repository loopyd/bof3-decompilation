#ifndef BOF3_CONTEXT_GAME_00_GLOBALS_H
#define BOF3_CONTEXT_GAME_00_GLOBALS_H

/* Global data declarations for the GAME_e00 overlay. */

/* Scratchpad work pointer (PS1 hardware register at 0x1F800044) */
#define SCRATCH_WORK    (*(struct GameWorkArea* volatile*)0x1F800044u)
#define SCRATCH_WORK_U8 (*(volatile u8**)0x1F800044u)

/* Global work pointer in main exe data section */
#define GLOBAL_WORK     (*(volatile u8**)0x80146250u)

/* Movement/position offset table accessors (table at 0x80180000+) */
#define MOVEMENT_TABLE_S32(base, index) (*(volatile s32*)((base) + (index)))
#define MOVEMENT_TABLE_S16(addr)        (*(volatile s16*)(addr))

#endif
