#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE40C
 * @behavior calls func_801DF6A8, then, when the global gate halfword
 *           D_80143C40 is clear, clears the UI sub-step byte D_80148652 and
 *           advances the UI phase byte D_80148651 by two; no path touches
 *           either byte when the gate is set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE40C(void) {
  func_801DF6A8();
  if (D_80143C40 == 0) {
    /* The original materializes one base for the phase byte and reuses it for
     * both the load and the store (lui+addiu / lbu / sb through v1); a volatile
     * access folds a fresh %hi/%lo pair per access instead (sibling shape:
     * func_801DF9B4). */
    PSX_REF(u8, (u32)&D_80148651) += 2;
    D_80148652 = 0;
  }
}
