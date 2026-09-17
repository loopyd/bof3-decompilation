#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 * @status partial
 * @match 81.25
 * @residual volatile signed-byte lowering adds sign-extension instruction; 64 bytes versus 60 original.
 */
void dispatchSecondaryState(void) {
  s8 state;

  state = D_80146874;
  secondaryStateTable[state]();
}
