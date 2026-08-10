#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2650
 * @behavior initializes shop UI state fields.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initializeShopUiState(void) {
  u8* base;

  base = D_80148330;
  D_80148331 = 7;
  D_80148332 = 0;
  D_80148333 = 2;
  base[0] = 1;
  base += 0x2F4;
  barrier();
  D_80148334 = 20;
  D_80148336 = -20;
  D_80148626 = 3;
  D_80148340 = 0;
  D_80148625 = 7;
  base[0] = 0;
  D_80148356 = 15;
  D_80148360 = 0x7F;
  D_80148358 = -100;
  D_80148355 = 7;
  D_80148357 = 2;
  D_8014835F = 0;
  D_8014835E = 0;
  D_80148361 = 0;
  D_8014835C = 0;
  D_8014835A = 62;
  base -= 0x2D0;
  *base = 1;
}
