#include "bof3/world/area03004_internal.h"

/* @behavior calls the shared mode helper func_8014ECAC with mode 1, stores
 * 0x1F in the shared byte D_8014832E, then increments byte 3 of the scratch
 * work record published at the scratchpad cursor 0x1F800044.
 * @source 0x801DDCB8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DDCB8(void) {
  u8* work;

  func_8014ECAC(1u);
  work = WORLD00_AREA030_SCRATCH_PTR;
  D_8014832E = 0x1fu;
  work[3] = (u8)(work[3] + 1u);
}
