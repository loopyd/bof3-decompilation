#include "bof3/world/area03213_internal.h"

/* @source 0x801F363C
 * @behavior Stores 0xFFFF at the shared flag D_801490A8; when the gate byte
 * D_801490C7 equals 1, publishes the D_80145E90 work base as the scratchpad
 * cursor at 0x1F800044, requests resource 0x80 through func_8014D6B8, and
 * restores the previously published cursor.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 25/25 instructions, 100 -> 100 bytes, exact on the first live
 * diff. The shared flag/gate pair mirrors the exact sibling func_801F36A0
 * (0x801F36A0): the gate byte load is hoisted above the 0x801490A8 store, and
 * the cursor is read through the named scratchpad cell before the work base is
 * published, so the restore keeps its value in $s0.
 */
void func_801F363C(void) {
  u8* previous;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 1) {
    previous = D_1F800044;
    D_1F800044 = D_80145E90;
    func_8014D6B8(0x80);
    D_1F800044 = previous;
  }
}
