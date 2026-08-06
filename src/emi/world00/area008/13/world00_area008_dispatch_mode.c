#include "internal.h"

/* @behavior dispatches the current scratchpad mode through the local handler
 * table.
 * @source 0x801F2C18
 */
void world00_area008_dispatch_mode(void) {
  u8 mode;

  mode = ((volatile u8*)WORLD00_AREA008_SCRATCH_PTR)[1];
  world00_area008_mode_handlerTable[mode]();
}
