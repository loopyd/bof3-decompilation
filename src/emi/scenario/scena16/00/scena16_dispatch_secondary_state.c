#include "internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 */
void scena16_dispatch_secondary_state(void) {
  s8 state;

  state = *(s8*)&SCENA16_D_80146874;
  scena16_secondary_stateTable[state]();
}
