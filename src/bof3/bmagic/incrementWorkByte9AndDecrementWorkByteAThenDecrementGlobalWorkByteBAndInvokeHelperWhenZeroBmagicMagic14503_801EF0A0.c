#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * every access group because a byte store through the loaded pointer may alias
 * it, so the compiler re-materializes `lui $vN,0x1F80` / `lw $vN,0x44($vN)`
 * exactly where the original does (twice in this body). Statement order, cell
 * spelling and declaration style follow the exact sibling
 * incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWorkByteBAnd
 * InvokeHelperWhenZeroBmagicMagic09803_801EF498 (0x801EF498), which keeps the
 * same two cell reloads, the same record byte 0x0B decrement and the same
 * helper call. */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The D_<ADDRESS> name binds with no map row; this overlay's remaining
 * assembly also loads the address. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_801E5988(void);

/* @source 0x801EF0A0
 * @behavior Per-frame handler of the battle work object published at the
 *           scratchpad pointer cell 0x1F800044. It increments that object's
 *           unsigned byte 0x09 by one at byte width (lbu / addiu 1 / sb),
 *           re-reads the cell and subtracts one from unsigned byte 0x0A the
 *           same way, then tests the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez) so the truncated register - not a
 *           reload - supplies the compared value. Only when byte 0x0A reaches
 *           zero does it read the shared battle work record pointer at
 *           0x801EB4E0, decrement that record's byte 0x0B by one and invoke the
 *           shared helper at 0x801E5988, the store filling the call delay slot.
 *           The first cell load precedes frame creation because the increment
 *           needs no stack; the 0x18-byte frame only keeps $ra across the call.
 *           Takes no arguments, returns nothing and reads or writes no other
 *           state. This 120-byte span is byte-identical to the still-asm rows
 *           of emi/bmagic/magic009/03 (0x801EF340), emi/bmagic/magic040/03
 *           (0x801EF1D0), emi/bmagic/magic059/03 (0x801EF5C0) and
 *           emi/bmagic/magic126/03 (0x801EFCE4), proven with one payload
 *           comparison; none of those rows has C yet, so the clone lever offered
 *           no source and this body was seeded from the sibling shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic14503_801EF0A0(void)
{
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] += 1;
  if (--slots[0x11][0x0A] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
