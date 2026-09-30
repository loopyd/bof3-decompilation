#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6668
 * @behavior reads the global gate halfword D_80143C40 once at entry; when it
 *           is clear, calls the this-target rebuild helper func_801D68D0 with
 *           the constant 0 and then increments the UI phase byte D_80148651,
 *           otherwise returns without touching either cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D6668(void) {
  if (D_80143C40 == 0) {
    func_801D68D0(0);
    /* The original materializes the phase byte's address once in $v1 and uses
     * it with offset 0 for both accesses; a volatile symbol access folds and
     * reloads instead (sibling shape: func_801DF9B4, same byte). */
    PSX_REF(u8, (u32)&D_80148651) += 1;
  }
}
