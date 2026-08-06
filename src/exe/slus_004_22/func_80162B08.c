#include "internal.h"

extern u32*             D_80146848;
extern u8               D_80146854;
extern EmiTransferSlot* D_80146844;
extern u32              D_80146678[];
extern volatile u32     emiExpectedSectorState; /* @kind: bss */
extern volatile u32     D_80146454;
extern volatile u32     D_80146458;
extern volatile u32     D_8014645C;
extern volatile u16     D_80146460;
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
    /*
     * MATCHING_AID:
     * The empty do/while and branch-local return reproduce the original
     * epilogue layout: with a shared trailing `return 1`, GCC cross-jumps
     * the two return tails and the delay-slot reorg pass duplicates
     * `li v0,1` into the zero-branch `j epilogue` slot (original: nop).
     * Returning 1 only here lets the zero branch reach the shared `jr ra`
     * with v0 already holding 1 from the `D_8014646C = 1` store.
     * Permuter-found (score 0). Remove if the compiler's tail-merge/
     * delay-slot behavior for this shape is understood structurally.
     */
    do {
    } while (0);
    return 1;
  }
}
