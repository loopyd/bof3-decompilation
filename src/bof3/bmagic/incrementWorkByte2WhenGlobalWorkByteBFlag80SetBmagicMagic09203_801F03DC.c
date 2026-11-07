#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4 guards on
 * the same byte 0xB, and
 * seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C seeds the
 * scratchpad work object behind the same guard). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F03DC
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 and, only when
 *           that record's byte at offset 0xB has bit 0x80 set (lbu / andi 0x80
 *           / beqz, so the original tests the masked byte rather than a value
 *           equality), reads the scratchpad work-object pointer cell at
 *           0x1F800044 once and increments its byte at offset 0x2 with a
 *           byte-width add (lbu / addiu / sb). The guard runs before the
 *           scratchpad cell is touched, so 0x1F800044 is only dereferenced on
 *           the taken path. Leaf with no frame; nothing is returned and no
 *           other state is read or written. This 68-byte body is byte-identical
 *           to 0x801F00F0 in emi/bmagic/magic144/03, which is still an unlifted
 *           raw row, so that twin is shape evidence rather than a clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBFlag80SetBmagicMagic09203_801F03DC(void) {
  if (D_801EB4E0[0xB] & 0x80) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
