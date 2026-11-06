#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * every access group because a byte store through the loaded pointer may alias
 * it, so the compiler re-materializes `lui $vN,0x1F80` / `lw $vN,68($vN)` exactly
 * where the original does (three times in this body). Statement order, cell
 * spelling and declaration style follow the exact sibling
 * incrementWorkBytes9AndAThenAdvanceWorkByte1At10BmagicMagic05503_801EEE3C
 * (0x801EEE3C), which reaches the same cell through SPAD_PTR_TABLE and keeps the
 * same three-group split. */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt binds the address, so no
 * target-map row is needed; the D_<ADDRESS> name auto-binds. The exact sibling
 * decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenZeroBmagicMagic15103_801F0CB0 stores through the same record byte 0xB and
 * reaches the same helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_801E5988(void);

/* @source 0x801EF498
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. It increments the object's unsigned byte
 *           0xB by one at byte width (lbu / addiu 1 / sb), re-reads the cell and
 *           subtracts two from unsigned byte 0xA the same way (lbu / addiu -2 /
 *           sb), then re-reads the cell once more and tests that byte for zero.
 *           Only when byte 0xA reaches zero does it read the shared battle work
 *           record pointer 0x801EB4E0, decrement that record's byte 0xB by one
 *           and invoke the shared helper at 0x801E5988, the store filling the
 *           call delay slot. Frame 0x18 with $ra only, because of the call; it
 *           takes no arguments, returns nothing and touches no other state.
 *
 *           Each of the three access groups keeps its own cell load: the
 *           original re-materialises the base before every group, so no pointer
 *           local may serve two of them. The zero test re-reads byte 0xA instead
 *           of truncating the stored value, which is why a plain u8 accumulator
 *           is enough here - the exact sibling
 *           decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvoke
 *           HelperWhenZeroBmagicMagic15103_801F0CB0 needed the s8 view only
 *           because its original compared the stored register after `sll 24`. This
 *           136-byte span is byte-identical to the still-asm rows of
 *           emi/bmagic/magic097/03 (0x801EF488), emi/bmagic/magic143/03
 *           (0x801EF100) and emi/bmagic/magic151/03 (0x801F1380), proven with one
 *           payload comparison; none of those rows has C yet, so the clone lever
 *           offered no source and this body was seeded from the sibling shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic09803_801EF498(void) {
    u8 **slots;

    slots = SPAD_PTR_TABLE(u8);

    slots[0x11][0x0B] += 1;
    slots[0x11][0x0A] -= 2;
    if (slots[0x11][0x0A] == 0) {
        u8 *record;

        record = D_801EB4E0;
        record[0x0B] -= 1;
        func_801E5988();
    }
}
