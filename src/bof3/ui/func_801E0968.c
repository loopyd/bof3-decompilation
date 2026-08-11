#include "bof3/ui/shop00_internal.h"

/* @source 0x801E0968
 * @behavior decrements phaseTimer; on wrap, initializes panel state and
 *           advances the UI phase byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E0968(void) {
  volatile u8* p = &phaseTimer;
  u8 val = *p - 1;

  *p = val;
  if (val == 0) {
    D_801483E5 = 7;
    D_801483E6 = 14;
    D_801483E4 = 1;
    D_801483EF = 0xFF;
    D_801483E8 = 200;
    D_801483EE = 0;
    D_801483EA = 63;
    D_80148652++;
  }
}
