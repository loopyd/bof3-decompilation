#include "bof3/battle/battle03_internal.h"

/**
 * @source 0x801E1050
 * @behavior Clear the current work's pending bit, reset two flags, and advance
 * the scratch-work state to 3.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E1050(void) {
  Battle03LocalWork* work;

  clearPendingBit(D_1F800044->unk_05);
  work = D_80146250;
  work->unk_124 &= ~0xF0u;
  work->unk_124 &= ~0x200u;
  D_1F800044->unk_01 = 3;
  D_1F800044->unk_02 = 0;
  D_1F800044->unk_03 = 0;
}
