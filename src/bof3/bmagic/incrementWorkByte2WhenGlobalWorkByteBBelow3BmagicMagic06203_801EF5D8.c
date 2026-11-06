#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay, already resolved by its
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF5D8
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 * compares its byte at offset 0xB against three with an unsigned bound
 * (lbu/sltiu), so the guard is taken for every value below three. Only on that
 * path it reads the scratchpad work pointer cell at 0x1F800044 once and
 * increments the byte at offset 2 modulo 256 with lbu/addiu/sb, advancing the
 * sub-step index that this overlay's 0x801EFBBC dispatcher indexes on. Bytes
 * 0x801EF5D8..0x801EF61B, entry 3 of the handler table at 0x801F0C10, share
 * this guard-and-increment shape with the sibling
 * incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EFD00, which
 * tests the same record byte for equality instead. Takes no arguments, returns
 * nothing and keeps no frame, so the scratchpad cell is dereferenced only when
 * the guard succeeds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic06203_801EF5D8(void) {
  if (D_801EB4E0[0xB] < 3u) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
