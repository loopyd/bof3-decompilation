#include "bof3/battle/battle03_internal.h"

/* @source 0x801DEC28
 * @behavior counts the source bytes up to the `arg3` limit, left-pads the
 * destination field with `(arg2 - count) / 2 + (count & 1)` `0xFF` bytes
 * starting at `arg0` + 1, copies up to `arg3` source bytes after them and
 * terminates the result with zero; destination byte 0 stays untouched.
 * @status partial
 * @match 93.48
 * @residual 43/46 instructions, size already exact (184 bytes); first mismatch +0x18 is the
 * scan-loop delay-slot fill: the original moves `addiu a1,a1,1` into the `beqz v0` exit-branch
 * delay slot and leaves the `bnez` back-edge slot `nop`, while this candidate lets reorg steal
 * the pad block's `andi v0,a2,0xff` into the exit slot and sinks `addiu a1,a1,1` into the
 * back-edge slot. Tried clean-C statement reorder and a load-tied `*arg1++` (0x801DEC28,
 * both regressed to 75.00%); one bounded `bin/permute --time-limit 60` found no improvement;
 * per-object scheduling/profile search is outside this lane. Next untried rung: pipeline-level
 * profile/`--expand-div`-class object flag evidence for this object.
 */
void centerFieldText(u8* arg0, u8* arg1, u8 arg2, u8 arg3) {
  u8* start;
  u8  count;
  u8  pad;
  u8  value;

  count = 0;
  start = arg1;
  if (arg3 != 0) {
    do {
      if (*arg1 == 0) {
        break;
      }
      arg1++;
      count++;
    } while (count < arg3);
  }

  pad = ((arg2 - count) / 2) + (count & 1);

  while (pad != 0) {
    arg0++;
    *arg0 = 0xff;
    pad--;
  }

  arg1 = start;
  count = 0;
  if (arg3 != 0) {
    do {
      value = *arg1;
      if (value == 0) {
        break;
      }
      *arg0 = value;
      arg1++;
      count++;
      arg0++;
    } while (count < arg3);
  }

  *arg0 = 0;
}
