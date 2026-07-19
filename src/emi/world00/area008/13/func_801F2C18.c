#include "internal.h"

/* @behavior dispatches the current scratchpad mode through the local handler
 * table.
 * @source 0x801F2C18
 */
void func_801F2C18(void) {
  World00Area008Handler* handlers;
  u8                     mode;

  mode = ((volatile u8*)WORLD00_AREA008_SCRATCH_PTR)[1];
  handlers = WORLD00_AREA008_HANDLER_TABLE;
  handlers[mode]();
}
