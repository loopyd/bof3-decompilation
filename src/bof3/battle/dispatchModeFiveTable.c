#include "bof3/battle/battle03_internal.h"

/* @behavior copies and dispatches the local five-entry battle-mode table.
 * @source 0x801E74B8; table words: 0x801E7528, 0x801E7558, 0x801E75B0,
 * 0x801E7634, 0x801E7778. The local targets have void(void) ABI.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchModeFiveTable(void) {
  Battle03FiveDispatchTable handlers;

  handlers = D_801D0F58;
  handlers.handlers[SPAD_PTR_SLOT(volatile u8, 0x44u)[1]]();
}
