#include "internal.h"

typedef struct EmiTransferSlot {
  u32 size;
  u32 remaining_size;
  u32 read_offset;
  u16 state;
} EmiTransferSlot;

extern u32*             DAT_80146848;
extern u8               DAT_80146854;
extern EmiTransferSlot* DAT_80146844;
extern u32              DAT_80146678[];
extern vu32             DAT_80146450;
extern vu32             DAT_80146454;
extern vu32             DAT_80146458;
extern vu32             DAT_8014645c;
extern vu16             DAT_80146460;
extern vu32             DAT_8014646c;

/* @behavior selects one pending EMI transfer slot and copies its staged transfer
 * state into the active loader registers.
 * @source 0x80162b08 FUN_80162b08
 */
s32 func_80162b08(u8 slot) {
  u8               slot_index;
  u32*             remaining_size;
  u32*             slot_size;
  EmiTransferSlot* slot_table;

  slot_index = slot;
  if ((*DAT_80146848 < slot_index) || (((DAT_80146854 & 0x20u) != 0) &&
                                       (DAT_80146844[slot_index].state < 6u))) {
    return 0;
  }

  if (slot_index != 0u) {
    DAT_80146450 = DAT_80146678[slot_index];
    slot_table = DAT_80146844 + slot_index;
    slot_size = &slot_table->size;
    DAT_80146454 = *slot_size;
    remaining_size = &slot_table->remaining_size;
    DAT_80146458 = *remaining_size;
    DAT_8014645c = slot_table->read_offset;
    DAT_8014646c = 0;
    DAT_80146460 = slot_table->state;
    return 1;
  }

  DAT_80146454 = 0x800;
  DAT_80146460 = 5;
  DAT_8014646c = 1;
  DAT_80146450 = DAT_80146678[0];
  return 1;
}
