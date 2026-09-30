#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * sibling lift incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4
 * reads the same cell, and the sibling battle lifts own the battle work record
 * (src/bof3/battle/func_801E8558_battle03.c reads it, runQueuedSlotHandlers.c
 * assigns it). Splat already binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFD80
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and, only
 * when that work record's byte at offset 0x1 equals 2, reads the scratchpad
 * work-object pointer cell at 0x1F800044 once and increments its byte at
 * offset 0x2 with a byte-width add (the original loads it with lbu, adds 1 and
 * stores it back with sb). Nothing is returned and no other state is read or
 * written; the guard compares against a materialized constant (addiu
 * $v0,$zero,2 then bne) and runs before the scratchpad cell is touched, so
 * 0x1F800044 is only dereferenced on the taken path. This is the same guard and
 * offset-0x2 increment shape as the sibling
 * incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4, which
 * tests that same work record's byte 0xB against 0x84.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByte1Is2BmagicMagic15103_801EFD80(void) {
  if (D_801EB4E0[1] == 2) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
