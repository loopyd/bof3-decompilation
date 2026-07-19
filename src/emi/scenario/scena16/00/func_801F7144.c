#include "internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 */
void func_801F7144(void) {
  s8 state;

  state = (s8)SCENA16_D_80146874;
  SCENA16_PTR_801F8558[state]();
}
