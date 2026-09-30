#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE3C8
 * @behavior when the global gate halfword D_80143C40 is clear, calls the
 *           main-exe frontend mode setter func_8014ECAC with the constant 3
 *           and then increments the UI sub-step byte D_80148652; when the gate
 *           is set neither the call nor the byte store happens.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE3C8(void) {
  if (D_80143C40 == 0) {
    func_8014ECAC(3);
    /* The original materializes the sub-step byte's address once in $v1 and
     * reuses it for the load and the store (lui+addiu / lbu / sb through v1);
     * a plain symbol access folds a fresh %lo offset per access instead
     * (sibling shape: func_801D6668, same counter family). */
    PSX_REF(u8, (u32)&D_80148652) += 1;
  }
}
