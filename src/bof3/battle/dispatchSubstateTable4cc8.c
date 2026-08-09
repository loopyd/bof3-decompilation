#include "bof3/battle/battle15_internal.h"

/*
 * @source 0x800A46C4
 * @behavior Dispatches the battle selection handler indexed by D_801462E4.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4cc8(void) {
  D_800B4CC8[D_801462E4]();
}
