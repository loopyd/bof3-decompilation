#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay, already resolved by its
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFCA0
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once. If its
 * byte at offset 0xB equals 0x82, reads the scratchpad work pointer at
 * 0x1F800044 once and increments its byte at offset 2 modulo 256. Takes no
 * arguments and returns nothing. The scratchpad pointer is read only when
 * the guard succeeds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs82BmagicMagic08703_801EFCA0(void) {
  if (D_801EB4E0[0xB] == 0x82) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
