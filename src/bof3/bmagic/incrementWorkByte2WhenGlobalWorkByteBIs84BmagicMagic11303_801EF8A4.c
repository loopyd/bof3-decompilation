#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * sibling battle lifts read and assign it as the battle work record
 * (src/bof3/battle/func_801E8558_battle03.c reads it, and
 * src/bof3/battle/runQueuedSlotHandlers.c assigns it). Splat already binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row or WEAK_SYMBOL_AT binding is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF8A4
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 and, only when
 * that work record's byte at offset 0xB equals 0x84, reads the scratchpad
 * work-object pointer cell at 0x1F800044 once and increments its byte at
 * offset 0x2 with a byte-width add (lbu/addiu/sb). Nothing is returned and no
 * other state is read or written; the guard runs before the scratchpad cell is
 * touched, so 0x1F800044 is only dereferenced on the taken path. Bytes
 * 0x801EF8A4..0x801EF8E3 are byte-identical to 0x801EFD00..0x801EFD3F in this
 * same overlay, which is the same guard and the same offset-0x2 increment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4(void) {
  if (D_801EB4E0[0xB] == 0x84) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
