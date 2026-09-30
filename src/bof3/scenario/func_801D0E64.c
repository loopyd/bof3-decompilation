#include "bof3/scenario/sce10eff_internal.h"

/* @behavior clears the shared scratchpad work-area header when byte 11 of the
 * scratchpad-resident state object equals 9.
 * @source 0x801D0E64
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D0E64(void) {
  u8* work;

  work = PSX_REF(u8*, SPAD_BASE + 0x44u);
  if (work[0x0b] == 9) {
    func_80196070();
  }
}
