#include "bof3/battle/battle15_internal.h"

/*
 * @source 0x80099328
 * @behavior Dispatches the selection-phase handler indexed by D_801462E4.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable447c(void) {
  D_800B447C[D_801462E4]();
}
