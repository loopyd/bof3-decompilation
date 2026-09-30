#include "bof3/battle/battle15_internal.h"

/* @source 0x8009CFEC
 * @behavior Compacts the local panel entry list: for each of the D_801EB5A8
 *   entries at D_801ED9B0 it searches the kept prefix for an entry whose
 *   0x118-byte owner record (D_801EB6AC, selected by the entry's owner index)
 *   has the same kind byte and the same panel id, drops the entry when one is
 *   found, otherwise appends it to the kept prefix; the kept count is written
 *   back to D_801EB5A8 and the kept entries are copied down over the list.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009CFEC(void) {
  BattleLocalPanelEntry buf[8];
  u8 i;
  u8 j;
  u8 n = 0;
  u8 found = 0;

  for (i = 0; i < D_801EB5A8; i++) {
    for (j = 0; j < n; j++) {
      if (D_801EB6AC[D_801ED9B0[i].owner_index].kind ==
              D_801EB6AC[buf[j].owner_index].kind &&
          D_801ED9B0[i].panel_id == buf[j].panel_id) {
        found = 1;
        break;
      }
    }
    if (found == 0) {
      buf[n] = D_801ED9B0[i];
      n++;
    }
    found = 0;
  }
  D_801EB5A8 = n;
  for (i = 0; i < D_801EB5A8; i++) {
    D_801ED9B0[i] = buf[i];
  }
}
