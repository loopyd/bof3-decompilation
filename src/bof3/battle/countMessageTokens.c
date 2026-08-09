#include "bof3/battle/battle03_internal.h"

/* @behavior counts tokens in a zero-terminated message stream, skipping one extra
 * byte after control codes `0x12`, `0x13`, and `0x15`.
 * @source 0x801EAB6C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 countMessageTokens(u8* arg0) {
  u8  count;
  u32 value;

  count = 0;
  value = *arg0;
  while (value != 0u) {
    if (((u32)(value - 0x12u) < 2u) || (value == 0x15u)) {
      arg0 += 1;
    }
    arg0 += 1;
    count += 1;
    value = *arg0;
  }
  return count;
}
