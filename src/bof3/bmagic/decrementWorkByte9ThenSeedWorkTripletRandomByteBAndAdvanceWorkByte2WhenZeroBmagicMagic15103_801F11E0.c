#include "bof3/bof3.h"

extern int rand(void);

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * (the asm handlers still reference it), so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use
 * (seedWorkTripletTintRandomWordCAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801F0BA8,
 * decrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2WhenZero
 * BmagicMagic15103_801EFD24): the symbolic address folds into each load (lui
 * $vN,0x1F80 / lw $vN,68($vN)), so the original's six separate reads each
 * re-materialise the base. The plain cell is kept rather than the volatile one
 * several siblings use: every read after the first is separated from the
 * previous one either by a store through the loaded pointer or by the rand()
 * call, so the reload comes from the store and not from a qualifier. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F11E0
 * @behavior Work-step handler of the overlay work object, published at word
 *           0x801F24FC of the code pointer run at 0x801F24A4 rather than reached
 *           by a jal. The scratchpad work-object pointer cell at 0x1F800044 is
 *           read once, before the 0x18-byte frame is created, and the
 *           work-object byte at offset 0x9 is decremented through that pointer
 *           with a byte-width read-modify-write (lbu / addiu -1 / sb); the
 *           truncated byte it just stored is tested (andi 0xFF of the stored
 *           register, no second cell load) and the function returns while that
 *           byte is still nonzero. On expiry it seeds a fresh step: the shared
 *           battle work record read through 0x801EB4E0 supplies its words 0x34,
 *           0x38 and 0x3C, which are copied into the work object through one
 *           further cell read, the 0x3C store filling the delay slot of the
 *           rand() call. rand()'s low four bits (andi 0xF) are then stored into
 *           work byte 0xB through a fresh cell read, work bytes 9 and 0xA are
 *           cleared and the state byte at offset 0x2 is incremented, each
 *           through its own cell read, advancing the overlay work byte 2
 *           handler sequence. Frame 0x18 with $ra only, because of the call; it
 *           takes no arguments, returns nothing and reads or writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSeedWorkTripletRandomByteBAndAdvanceWorkByte2WhenZeroBmagicMagic15103_801F11E0(void) {
    u8 *work;
    u8 countdown;
    s32 *source;
    s32 *dest;
    s32 noise;
    u8 *setB;
    u8 *clear9;
    u8 *clearA;
    u8 *step;

    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
        source = (s32 *)D_801EB4E0;
        dest = (s32 *)g_battle_work;
        dest[13] = source[13];
        dest[14] = source[14];
        dest[15] = source[15];
        noise = rand() & 0xF;
        setB = g_battle_work;
        setB[0xB] = noise;
        clear9 = g_battle_work;
        clear9[9] = 0;
        clearA = g_battle_work;
        clearA[0xA] = 0;
        step = g_battle_work;
        step[2]++;
    }
}
