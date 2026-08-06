#include "internal.h"

/* @behavior dispatches through the primary SCENA16 state table.
 * @source 0x801F6C90
 */
void scena16_dispatch_primary_state(void) {
  s8* state_base;

  state_base = PSX_PTR(s8, 0x80140000u);
  scena16_primary_stateTable[state_base[0x6872]]();
}
