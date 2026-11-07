#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is the named cell
 * symbol the exact siblings of this target use: the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), and the original re-materialises
 * it before every store group because the stores through the previously loaded
 * pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF448
 * @behavior Per-frame work-handler step, entry 8 of the sixteen-word code pointer
 *           run at 0x801F0D54 whose word at 0x801F0D74 holds this function's
 *           address. It reads the scratchpad work object through the pointer
 *           cell at 0x1F800044 and decrements its unsigned byte 9 in place
 *           (lbu / addiu -1 / sb / andi 0xFF / bnez), so the rest of the body
 *           runs only when that countdown reaches zero. On zero it re-reads the
 *           cell and raises bit 0x20 of work byte 0 (the same flag the exact
 *           siblings seedWorkObjectFromBattleRecordAndAdvanceWorkByte2
 *           BmagicMagic04500_801EF4A4 and decrementWorkCountdownThenSeedWork
 *           TintFromByteBRefreshWorkFromRecordAndAdvanceWorkByte2Bmagic
 *           Magic01500_801EFFC0 raise), re-reads the cell and sets mode byte
 *           0x5C to 1, re-reads the cell before each of the three tint-channel
 *           bytes it clears (0x5D, 0x5E and 0x5F), and re-reads the cell once
 *           more to advance work byte 2 by one, which moves the dispatcher to
 *           the next byte-2 step. Each store re-materialises the cell because a
 *           store through the loaded pointer may alias it; the three zero stores
 *           leave their load delay slots unfilled, while the byte-0x5C store
 *           fills its slot with the constant 1. Leaf with no frame; takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           168-byte span is byte-for-byte identical to func_801EF5F0 of
 *           emi/bmagic/magic226/03, whose map row is still asm, so no extracted
 *           payload carries a lifted copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSetFlag20Mode1ClearTintTripletAndAdvanceWorkByte2BmagicMagic22203_801EF448(void) {
    if (--g_battle_work[9] == 0) {
        g_battle_work[0] |= 0x20;
        g_battle_work[0x5C] = 1;
        g_battle_work[0x5D] = 0;
        g_battle_work[0x5E] = 0;
        g_battle_work[0x5F] = 0;
        g_battle_work[2]++;
    }
}
