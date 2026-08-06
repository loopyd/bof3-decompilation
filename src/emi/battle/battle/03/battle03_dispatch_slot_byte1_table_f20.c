#include "internal.h"

/* @behavior dispatches the queued-slot selected handler from the local three-entry table.
 * @source 0x801E63C0
 */
void battle03_dispatch_slot_byte1_table_f20(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0F20;
  handlers.handlers[((volatile u8*)D_801EC2E0)[1]]();
}
