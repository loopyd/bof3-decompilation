#include "internal.h"

/* @behavior dispatches through the secondary SCENA16 state table.
 * @source 0x801F7144
 */
void func_801F7144(void) {
  s8 state;

  state = *(s8*)0x80146874u;
  SCENA16_PTR_801F8558[state]();
}
