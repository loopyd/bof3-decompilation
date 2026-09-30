#include "bof3/ui/shop00_internal.h"

/* @source 0x801E1D84
 * @behavior orders the main-RAM 128-slot skill-notes bank D_8014546C by
 *           ability cost after normalizing it; it is the mirror twin of the
 *           sibling sortSkillNotesByCostDescending with the cost comparison
 *           reversed. It first runs the scanner func_801E1BCC over the bank,
 *           then for every outer pass i in 0..0x7E re-seeds the byte cursor p
 *           at the bank start and walks j from 0 while j is below the signed
 *           pass limit 0x7F - i, deriving the next byte address q from the
 *           current cursor on every step and swapping the pair through
 *           swapBytes when both bytes are non-zero and the cost byte at offset
 *           0x0E of the 0x14-byte ability record selected by p (so
 *           abilityObjects[kind].cost, the byte at 0x801CA71A) exceeds the cost
 *           of the record selected by q, then advancing the cursor by one
 *           byte. Both counters are u8 and the cost comparison is unsigned, so
 *           the pass limit is recomputed from the truncated pass counter and
 *           the q address is re-derived from the cursor on every step. Moving
 *           the smaller cost one slot earlier per pass orders the non-zero
 *           prefix of the bank by ascending cost. Returns void; the command id
 *           handed over by the shop command handler table is not read.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E1D84(u32 arg0) {
  u8* p;
  u8* q;
  u8 i;
  u8 j;

  func_801E1BCC();
  for (i = 0; i < 0x7F; i++) {
    p = D_8014546C;
    for (j = 0; j < 0x7F - i; j++) {
      q = p + 1;
      if (*p != 0 && *q != 0 &&
          abilityObjects[*p].cost > abilityObjects[*q].cost) {
        swapBytes(p, q);
      }
      p++;
    }
  }
}
