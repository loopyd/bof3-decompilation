#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 * @status invalid
 * @match 100.00
 * @residual byte-exact but the state read casts away D_80146874 volatility; requires semantic repair.
 */
void dispatchSecondaryState(void) {
  s8 state;

  state = *(s8*)&D_80146874;
  secondaryStateTable[state]();
}
