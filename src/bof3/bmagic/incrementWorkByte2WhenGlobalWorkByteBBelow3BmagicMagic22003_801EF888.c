#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay, already resolved by its
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF888
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 * compares its byte at offset 0xB against three with an unsigned bound
 * (lbu/sltiu), so the guard is taken for every value below three. Only on that
 * path it reads the scratchpad work pointer cell at 0x1F800044 once and
 * increments the byte at offset 2 modulo 256 with lbu/addiu/sb. Takes no
 * arguments, returns nothing and keeps no frame, so the scratchpad cell is
 * dereferenced only when the guard succeeds. Bytes 0x801EF888..0x801EF8CB are
 * byte-identical to the exact siblings
 * incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic06203_801EF5D8 and
 * incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic08203_801EF680.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic22003_801EF888(void) {
  if (D_801EB4E0[0xB] < 3u) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
