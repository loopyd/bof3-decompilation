#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay, already resolved by its
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFBA8
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 *           compares its byte at offset 0xB against two with an unsigned bound
 *           (lbu/sltiu), so the guard is taken for every value below two. Only
 *           on that path it reads the scratchpad work pointer cell at
 *           0x1F800044 once and increments the byte at offset 2 of that work
 *           object modulo 256 with lbu/addiu/sb, sharing the loaded base for
 *           the read and the store. Takes no arguments, returns nothing and
 *           keeps no frame, so the scratchpad cell is dereferenced only when
 *           the guard succeeds. Bytes 0x801EFBA8..0x801EFBEB are entry 2 of
 *           the five-entry pointer run at 0x801EFE58 (0x801EF910, 0x801EFAF4,
 *           0x801EFBA8, 0x801EFBEC, 0x801EFD84), so the body has no jal caller
 *           in the payload. They are byte-identical to the exact lift
 *           0x801EF5EC of bmagic/magic088/03, whose source is cloned here with
 *           only the symbol and file name changed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow2BmagicMagic00403_801EFBA8(void) {
  if (D_801EB4E0[0xB] < 2u) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
