#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell. This body reads the cell once, so the
 * single access stays one plain pointer load and the target map binds the
 * name. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The word-typed declaration and the derived byte view follow this target's
 * other sources; the target map binds the name. */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0CE4
 * @behavior Entry index 2, the last word, of the three-entry code-pointer run
 *           at 0x801F1C24 that 0x801F0B44 indexes with the scratchpad work
 *           object's byte 2 scaled by four. Reads the work-object pointer cell
 *           at 0x1F800044 once and performs two read-modify-writes through that
 *           single loaded pointer: it decrements the work-object byte at offset
 *           0x9 at byte width (lbu / addiu -2 / sb) and advances the work-object
 *           word at offset 0x14 by 0x10, whose store the original leaves for
 *           the branch delay slot. The byte the original compares is the stored
 *           register truncated to eight bits (andi $v1,$v1,0xFF), so no second
 *           cell read supplies the guard. Only when that byte reaches zero does
 *           it read the shared battle work record pointer at 0x801EB4E0,
 *           decrement that record's byte 0x0B by one and invoke the shared
 *           helper at 0x801E5988, the store filling the call delay slot. The
 *           cell load precedes frame creation; the 0x18-byte frame only keeps
 *           $ra at 0x10 across the call. Takes no arguments, returns nothing
 *           and reads or writes no other state. A payload comparison measured
 *           this 0x60-byte body equal to the body lifted at
 *           emi/bmagic/magic151/03 0x801F1D60 in every word but its two
 *           immediates (the byte decrement and the word 0x14 increment), which
 *           places this body's discriminating immediates in this target's
 *           payload alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9By2AdvanceWorkWord14By10ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic13103_801F0CE4(void) {
    u8 *work;
    s32 value;

    work = g_battle_work;
    value = *(s32 *)(work + 0x14) + 0x10;
    *(s32 *)(work + 0x14) = value;
    if ((work[0x9] -= 2) == 0) {
        ((u8 *)record801EB4E0)[0xB] -= 1;
        func_801E5988();
    }
}
