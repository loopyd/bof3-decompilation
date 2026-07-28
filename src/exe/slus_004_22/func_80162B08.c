#include "internal.h"

extern u32*             D_80146848;
extern u8               D_80146854;
extern EmiTransferSlot* D_80146844;
extern u32              D_80146678[];
extern volatile u32     D_80146450;
extern volatile u32     D_80146454;
extern volatile u32     D_80146458;
extern volatile u32     D_8014645C;
extern volatile u16     D_80146460;
extern u32              D_8014646C;

/* @behavior selects one pending EMI transfer slot and copies its staged transfer
 * state into the active loader registers.
 * @source 0x80162B08
 *
 * RESIDUAL: bin/byte-match exits non-zero. 66/67 instructions match (98.51%);
 * byte size matches (268--268). The sole difference: the `j epilogue` delay slot
 * contains `li v0,1` (current) vs `nop` (original). Canonical -O2 output differs.
 * Flag-search (52 catalog variants): no exact match; best result 98.51%.
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
