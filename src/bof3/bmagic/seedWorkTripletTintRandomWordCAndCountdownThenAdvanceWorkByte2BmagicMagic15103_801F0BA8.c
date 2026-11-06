#include "bof3/bof3.h"

extern int rand(void);

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * (func_801F1D60 still references it), so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Kept as the named cell symbol the exact siblings on this target use
 * (seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801EFE90, copyWorkTripletSetByte9To10AndWord14To100
 * ThenAdvanceWorkByte2BmagicMagic15103_801F1CCC): the symbolic address folds
 * into each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's six
 * separate reads each re-materialise the base. The plain cell is kept rather
 * than the volatile one several siblings use: every read after the first is
 * separated from the previous one by a store through the loaded pointer, so the
 * reload comes from the store and not from a qualifier. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0BA8
 * @behavior Seeds one work step from the shared battle work record read through
 *           0x801EB4E0: its words 0x34, 0x38 and 0x3C are copied into the
 *           scratchpad work object published at the pointer cell 0x1F800044
 *           through one cell read, the 0x3C store filling the call's delay
 *           slot. rand() is then called and its low five bits (andi 0x1F) are
 *           stored into work word 0xC through a second cell read, the same
 *           pointer that stores 0x10 into work byte 0x5D (the tint channel the
 *           exact sibling 0x801EFE90 seeds). Work byte 0xB and work byte 9 are
 *           cleared and work byte 0xA is set to 4, each through its own cell
 *           read, and the trailing byte 2 increment re-reads the cell once more
 *           with its store in the return delay slot. Frame 0x18 with $ra only,
 *           because of the call; nothing is returned and no other state is read
 *           or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkTripletTintRandomWordCAndCountdownThenAdvanceWorkByte2BmagicMagic15103_801F0BA8(void) {
    s32 *source;
    s32 *work;
    s32 noise;
    u8 *tint;
    u8 *clearB;
    u8 *clear9;
    u8 *setA;
    u8 *step;

    source = (s32 *)D_801EB4E0;
    work = (s32 *)g_battle_work;
    work[13] = source[13];
    work[14] = source[14];
    work[15] = source[15];
    noise = rand() & 0x1F;
    tint = (u8 *)g_battle_work;
    tint[0x5D] = 0x10;
    ((s32 *)tint)[3] = noise;
    clearB = g_battle_work;
    clearB[0xB] = 0;
    clear9 = g_battle_work;
    clear9[9] = 0;
    setA = g_battle_work;
    setA[0xA] = 4;
    step = g_battle_work;
    step[2]++;
}
