#include "internal.h"

/* does: dispatches through the secondary SCENA16 state table.
 * @source: 0x801f7144 FUN_801f7144
 */
void func_801f7144(void) {
  s8 state;

  state = *(s8*)0x80146874u;
  BOF3_SCENA16_PTR_FUN_801f8558[state]();
}
