#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the queued-slot selected handler from the local three-entry table.
 * @source 0x801E63C0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSlotByte1TableF20(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0F20;
  handlers.handlers[((volatile u8*)D_801EC2E0)[1]]();
}
