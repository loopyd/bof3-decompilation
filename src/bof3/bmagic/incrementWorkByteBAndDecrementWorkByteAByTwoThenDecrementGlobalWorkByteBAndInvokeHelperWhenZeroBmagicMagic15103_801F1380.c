#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)). The cell is re-evaluated for
 * every access group because a byte store through the loaded pointer may alias
 * it, so the compiler re-materializes the base exactly where the original does
 * (three times in this body). The plain cell is kept rather than the volatile
 * one several siblings use: every read after the first is already separated
 * from the previous one by a store through the loaded pointer, so the reload
 * comes from the store and not from a qualifier. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * no target-map row is needed. The exact sibling
 * decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenZeroBmagicMagic15103_801F0CB0 stores through the same record byte 0xB and
 * reaches the same helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_801E5988(void);

/* @source 0x801F1380
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. The cell load for the first group is
 *           emitted ahead of the frame setup, as in the exact sibling
 *           decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAnd
 *           InvokeHelperWhenZeroBmagicMagic15103_801F0CB0. It increments the
 *           object's unsigned byte 0xB by one at byte width (lbu / addiu 1 /
 *           sb), re-reads the cell and subtracts two from unsigned byte 0xA the
 *           same way (lbu / addiu -2 / sb), then re-reads the cell once more and
 *           tests that byte for zero. Only when byte 0xA reaches zero does it
 *           read the shared battle work record pointer 0x801EB4E0, decrement
 *           that record's byte 0xB by one and invoke the shared helper at
 *           0x801E5988, the store filling the call delay slot. Frame 0x18 with
 *           $ra only, because of the call; it takes no arguments, returns
 *           nothing and touches no other state.
 *
 *           Each of the three access groups keeps its own cell load: the
 *           original re-materialises the base before every group, so no pointer
 *           local may serve two of them. The zero test re-reads byte 0xA instead
 *           of truncating the stored value, which is why a plain u8 accumulator
 *           is enough here - the exact sibling
 *           decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAnd
 *           InvokeHelperWhenZeroBmagicMagic15103_801F0CB0 needed the s8 view only
 *           because its original compared the stored register after sll 24.
 *           The cell spelling and the three-group split follow the exact
 *           sibling incrementWorkByteBAdd4ToWorkByteAThenSetWorkByte9To8And
 *           AdvanceWorkByte2At10BmagicMagic15103_801F129C on this same cell.
 *
 *           This 136-byte span is byte-identical to the already-exact source of
 *           emi/bmagic/magic098/03 (0x801EF498), proven with one payload
 *           comparison, and equally byte-identical to the still-asm rows of
 *           emi/bmagic/magic097/03 (0x801EF488) and emi/bmagic/magic143/03
 *           (0x801EF100). The clone adapter was declined because that
 *           representative reaches the cell through SPAD_PTR_TABLE, which this
 *           target does not declare, and because this target already binds the
 *           cell as g_battle_work; the body is the shape copy of that sibling
 *           with this target's evidenced cell spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F1380(void) {
    g_battle_work[0x0B] += 1;
    g_battle_work[0x0A] -= 2;
    if (g_battle_work[0x0A] == 0) {
        u8 *record;

        record = D_801EB4E0;
        record[0x0B] -= 1;
        func_801E5988();
    }
}
