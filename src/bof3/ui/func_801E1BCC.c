#include "bof3/ui/shop00_internal.h"

/* @source 0x801E1BCC
 * @behavior normalizes adjacent byte pairs of the main-RAM skill-notes bank
 *           D_8014546C: for every outer pass i in 0..0x7E it re-seeds the
 *           byte cursor p at the bank start and walks j from 0 while j is
 *           below the signed pass limit 0x7F - i; each step derives the next
 *           byte address q from the current cursor and, when the byte at p is
 *           zero and the byte at q is not, swaps the pair through
 *           swapBytes, then advances the cursor by one byte. Both counters
 *           are u8, so the pass limit is recomputed from the truncated
 *           pass counter, and the q address is re-derived from the cursor on
 *           every step. Returns void.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E1BCC(void) {
  u8* p;
  u8* q;
  u8 i;
  u8 j;

  for (i = 0; i < 0x7F; i++) {
    p = D_8014546C;
    for (j = 0; j < 0x7F - i; j++) {
      q = p + 1;
      if (*p == 0 && *q != 0) {
        swapBytes(p, q);
      }
      p++;
    }
  }
}
