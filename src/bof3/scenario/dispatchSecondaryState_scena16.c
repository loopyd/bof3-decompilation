#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSecondaryState(void) {
  s8 state;

  state = *(s8*)&D_80146874;
  secondaryStateTable[state]();
}
