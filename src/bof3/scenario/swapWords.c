#include "bof3/bof3.h"

/* @source 0x801D2138
 * @behavior Exchanges the two 32-bit words addressed by arg0 and arg1 in
 *           place with the xor swap triple: arg0 ^= arg1, then arg1 ^= arg0,
 *           then arg0 ^= arg1, so each pointer is re-read after the store that
 *           could have changed it. Every caller hands it two word addresses
 *           inside the scratchpad work record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void swapWords(u32 *arg0, u32 *arg1) {
  *arg0 = *arg0 ^ *arg1;
  *arg1 = *arg1 ^ *arg0;
  *arg0 = *arg0 ^ *arg1;
}
