#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm spans func_801EF148, func_801EF4F0, func_801EFF7C,
 * func_801F0070 and func_801F0584 still reference it, so this lift needs no
 * target-map row. The offsets read here (words 0x34, 0x38 and 0x3C) are the ones
 * the exact bmagic siblings
 * copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * and copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344
 * copy from the record they read through 0x801EB4E0; the record layout is
 * shared, the address is this target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and shared by this overlay's dispatchers and handlers. Declared as a named
 * cell symbol, the form the exact siblings use: the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)) and the original re-materialises
 * it before every later store group, because the stores through the loaded
 * pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF634
 * @behavior Per-frame step of a work-object handler this overlay dispatches on
 *           a work-object byte. Reads the scratchpad work object through the
 *           pointer cell at 0x1F800044 and decrements its unsigned byte 9 in
 *           place (lbu / addiu -1 / sb / andi 0xFF / bnez), so the rest of the
 *           body runs only when that countdown reaches zero. On zero it reads
 *           the battle record published at 0x801EB4E0, re-reads the work cell,
 *           copies the record's words 0x34, 0x38 and 0x3C into the work
 *           object's words 0x34, 0x38 and 0x3C, clears the work word 0x14 and
 *           the work byte 9 there (the word 0x3C is held in a local so its load
 *           precedes those two stores and its store follows them), re-reads the
 *           cell to set work byte 0xA to 0x26, and re-reads the cell once more
 *           to advance work byte 2 by one, which moves the dispatcher to the
 *           next work-byte-2 step. Leaf with no frame; takes no arguments,
 *           returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyWorkWordTripletClearWord14AndByte9SetATo26AndAdvanceWorkByte2BmagicMagic22203_801EF634(void) {
    u32 *source;
    u32 *work;
    u32 value;

    if (--g_battle_work[9] == 0) {
        source = (u32 *)D_801EB4E0;
        work = (u32 *)g_battle_work;
        work[13] = source[13];
        work[14] = source[14];
        value = source[15];
        work[5] = 0;
        ((u8 *)work)[9] = 0;
        work[15] = value;
        g_battle_work[0xA] = 0x26;
        g_battle_work[2]++;
    }
}
