#include "internal.h"

typedef struct EmiTransferSlot {
  u32 size;
  u32 remaining_size;
  u32 read_offset;
  u16 state;
} EmiTransferSlot;

extern u32*             D_80146848;
extern u8               D_80146854;
extern EmiTransferSlot* D_80146844;
extern u32              D_80146678[];
extern vu32             D_80146450;
extern vu32             D_80146454;
extern vu32             D_80146458;
extern vu32             D_8014645C;
extern vu16             D_80146460;
extern u32              D_8014646C;

/* @behavior selects one pending EMI transfer slot and copies its staged transfer
 * state into the active loader registers.
 * @source 0x80162B08
 */
s32 func_80162B08(u8 slot) {
  volatile EmiTransferSlot* slot_table;

  if ((*D_80146848 < slot) ||
      (((D_80146854 & 0x20u) != 0) && (D_80146844[slot].state < 6u))) {
    return 0;
  }

  if (slot == 0u) {
    D_80146454 = 0x800;
    D_80146460 = 5;
    D_8014646C = 1;
    D_80146450 = D_80146678[0];
  } else {
    D_80146450 = D_80146678[slot];
    /* Preserve the original compiler's slot-address register allocation. */
    slot_table = D_80146844 - -slot;
    D_80146454 = slot_table->size;
    D_80146458 = slot_table->remaining_size;
    D_8014645C = slot_table->read_offset;
    D_8014646C = 0;
    D_80146460 = slot_table->state;
  }

  return 1;
}
