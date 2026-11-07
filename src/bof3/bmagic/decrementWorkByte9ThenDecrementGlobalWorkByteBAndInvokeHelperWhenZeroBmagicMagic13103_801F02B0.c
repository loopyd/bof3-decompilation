#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This body reads the cell once, so the single access stays one plain pointer
 * load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The word-typed declaration and the derived byte view follow this target's
 * other sources; the target map binds the name. */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F02B0
 * @behavior Entry index 4, the last word, of the five-entry work-byte-2 step
 *           table at 0x801F1BF4 that 0x801F0100 indexes with the scratchpad work
 *           object's byte 2. Decrements the work object's countdown byte 0x09 at
 *           byte width (lbu / addiu -1 / sb) and tests the stored byte truncated
 *           to eight bits (andi $v0,$v0,0xFF / bnez), so the truncated register -
 *           not a reload - supplies the compared value. Only when the countdown
 *           reaches zero does it read the shared battle work record pointer at
 *           0x801EB4E0, decrement that record's byte 0x0B by one and invoke the
 *           shared helper at 0x801E5988, the store filling the call delay slot.
 *           The pointer-cell load precedes frame creation; the 0x18-byte frame
 *           only keeps $ra at 0x10 across the call. Takes no arguments, returns
 *           nothing and reads or writes no other state. The 92-byte body is
 *           byte-identical to the still-asm spans at emi/bmagic/magic006/03
 *           0x801EF41C, magic041/03 0x801EF564, magic060/00 0x801F00D0,
 *           magic062/03 0x801EF61C, magic067/03 0x801F0068, magic076/03
 *           0x801EF63C, magic082/03 0x801EF6C4, magic088/03 0x801EF630,
 *           magic110/03 0x801EF93C, magic111/03 0x801EF2D4, magic126/03
 *           0x801EF0BC, magic137/03 0x801EFD9C, magic167/03 0x801EF454,
 *           magic222/03 0x801EF770 and magic223/03 0x801EF18C, proven with one
 *           payload comparison; none of those rows has authored C yet, so the
 *           clone lever offered no source and this body was seeded from the
 *           sibling shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic13103_801F02B0(void) {
  if (--g_battle_work[0x09] == 0) {
    ((u8 *)record801EB4E0)[0x0B] -= 1;
    func_801E5988();
  }
}
