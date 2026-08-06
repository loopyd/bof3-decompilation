#include "internal.h"

/* @behavior dispatches the queued-slot selected handler from the local three-entry table.
 * Table bytes at 0x801D0F2C are 0x801E6990, 0x801E69C0, and 0x801E6A54;
 * each target consumes no incoming argument, so the local ABI is void(void).
 * @source 0x801E6930
 */
void battle03_dispatch_slot_byte1_table_f2c(void) {
  Battle03DispatchTable handlers;

  handlers = D_801D0F2C;
  handlers.handlers[((volatile u8*)D_801EC2E0)[1]]();
}
