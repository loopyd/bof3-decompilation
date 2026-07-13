#include "internal.h"

/* @behavior dispatches the current scratchpad mode through the local handler
 * table.
 * @source 0x801f2c18 func_801f2c18
 */
void func_801f2c18(void) {
  void (**handlers)(void);
  u8 mode;

  mode = ((volatile u8*)WORLD00_AREA008_SCRATCH_PTR)[1];
  handlers = (void (**)(void))0x801f4688u;
  handlers[mode]();
}
