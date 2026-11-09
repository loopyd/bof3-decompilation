#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and shared by the overlay's dispatchers and handlers. Kept as the named cell
 * symbol the exact siblings on this target use, which folds the symbolic address
 * into every load (lui $vN,0x1F80 / lw $vN,68($vN)). The cell is re-evaluated
 * for all three access groups because the byte store through the loaded pointer
 * may alias it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * the D_<ADDRESS> spelling needs no target-map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_801E5988(void);

/* @source 0x801EF100
 * @behavior Handler of the work object published at the scratchpad pointer cell
 *           0x1F800044. It increments that object's unsigned byte 0xB by one at
 *           byte width (lbu / addiu 1 / sb), then subtracts two from unsigned
 *           byte 0xA the same way (lbu / addiu -2 / sb), then re-reads byte 0xA
 *           from the cell and tests it for zero. Only when the countdown reaches
 *           zero does it read the shared battle work record pointer at
 *           0x801EB4E0, decrement that record's byte 0xB by one and invoke the
 *           shared helper at 0x801E5988, whose store fills the call delay slot.
 *           The 0x18-byte frame exists only to keep $ra across that call; the
 *           function takes no arguments, returns nothing and touches no other
 *           state, and the equal-and-branch is the last decision in the body, so
 *           the return path is the single shared epilogue at 0x801EF178. The
 *           cell load for the first group is emitted ahead of the frame setup,
 *           as in the exact sibling
 *           incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWork
 *           ByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F1380.
 *
 *           The span 0x801EF100..0x801EF187 (136 bytes, 34 instructions) ends
 *           exactly at the delay-slot nop before the next function at
 *           0x801EF188, so it needs no carve and holds no data. It is
 *           byte-identical (sha256
 *           33115358b3638c2cf00e41ec8a494b999c16eb5f598484ce88ec2af84de8ecb2) to
 *           the payload spans of the exact sources
 *           incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWork
 *           ByteBAndInvokeHelperWhenZeroBmagicMagic09703_801EF488 and
 *           incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWork
 *           ByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F1380; the clone
 *           adapter declined the range because it cannot respell the
 *           representative's SPAD_PTR_TABLE cell, so the body copies the
 *           magic15103 statement shape onto this target's evidenced cell
 *           spelling.
 *
 *           The address is entry 4 (the last word) of the five-word in-image
 *           handler run at 0x801EFEE8 (0x801EEDB4, 0x801EEF10, 0x801EEF78,
 *           0x801EF054, 0x801EF100); no jal in the payload reaches it, and the
 *           row is dispatched as table index 4 of work byte 1 by
 *           dispatchStateByte1Tablefee8BmagicMagic14303_801EED70.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndDecrementWorkByteAByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic14303_801EF100(void) {
  g_battle_work[0x0B] += 1;
  g_battle_work[0x0A] -= 2;
  if (g_battle_work[0x0A] == 0) {
    u8 *record;

    record = D_801EB4E0;
    record[0x0B] -= 1;
    func_801E5988();
  }
}
