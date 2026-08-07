#include "internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 */
void dispatchSecondaryState(void) {
  s8 state;

  state = *(s8*)&D_80146874;
  secondaryStateTable[state]();
}
