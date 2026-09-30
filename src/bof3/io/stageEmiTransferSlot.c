#include "bof3/core/slus_internal.h"

/* @behavior selects one pending EMI transfer slot and copies its staged transfer
 * state into the active loader registers.
 * @source 0x80162B08
 * @status exact
 * @match 100.00
 * @residual none
 */

extern u32*             D_80146848;
extern u8               D_80146854;
extern EmiTransferSlot* D_80146844;
extern u32              D_80146678[];
/* @source 0x80146450 @kind bss */
extern volatile u32     emiExpectedSectorState;
extern volatile u32     D_80146454;
extern volatile u32     D_80146458;
extern volatile u32     D_8014645C;
extern volatile u16     D_80146460;
extern u32              D_8014646C;

s32 stageEmiTransferSlot(u8 slot) {
  volatile EmiTransferSlot* slot_table;
  if ((*D_80146848 < slot) ||
      (((D_80146854 & 0x20u) != 0) && (D_80146844[slot].state < 6u))) {
    return 0;
  }

  if (slot == 0u) {
    D_80146454 = 0x800;
    D_80146460 = 5;
    D_8014646C = 1;
    emiExpectedSectorState = D_80146678[0];
  } else {
    emiExpectedSectorState = D_80146678[slot];
    /* Preserve the original compiler's slot-address register allocation. */
    slot_table = D_80146844 - -slot;
    D_80146454 = slot_table->size;
    D_80146458 = slot_table->remaining_size;
    D_8014645C = slot_table->read_offset;
    D_8014646C = 0;
    D_80146460 = slot_table->state;
    do {
    } while (0);
    return 1;
  }
}
