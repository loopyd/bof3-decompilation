#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches through the primary SCENA16 state table.
 * @source 0x801F6C90
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchPrimaryState(void) {
  s8* state_base;

  state_base = PSX_PTR(s8, 0x80140000u);
  primaryStateTable[state_base[0x6872]]();
}
