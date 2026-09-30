#include "bof3/world/area03004_internal.h"

/* @behavior when the shared mode byte is clear, calls the shared mode helper
 * func_8014ECAC with mode 0 and increments byte 3 of the scratch work record
 * published at the scratchpad cursor 0x1F800044.
 * @source 0x801DDC0C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DDC0C(void) {
  u8* work;

  if (modeByte == 0u) {
    func_8014ECAC(0u);
    work = WORLD00_AREA030_SCRATCH_PTR;
    work[3] = (u8)(work[3] + 1u);
  }
}
