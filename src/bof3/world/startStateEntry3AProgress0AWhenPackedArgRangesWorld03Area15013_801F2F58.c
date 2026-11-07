#include "bof3/bof3.h"

extern u8 D_801448EB;
extern u8 D_801448EC;

void func_8015C088(void);

/* @source 0x801F2F58
 * @behavior Overlay entry that starts shared state entry 0x3A at progress 0x0A
 *   when both packed arguments match. It sign-extends the high half of the
 *   first argument (`sra $a0,$a0,16`) and returns 0 unless that half lies in
 *   0x17..0x1C (the `addiu -0x17` / `sltiu 6` pair), then does the same for the
 *   second argument against 0x33..0x38 (`sra $v0,$a1,16`, `addiu -0x33`,
 *   `sltiu 6`) and returns 0 when that also fails. On a match it runs the shared
 *   front-end startup helper func_8015C088() and then stores 0x3A into the state
 *   selector D_801448EB and 0x0A into the progress byte D_801448EC, returning 1.
 *   The 0x18-byte frame only keeps $ra across the call: the $ra store fills the
 *   first range test's branch delay slot, each byte store materialises the
 *   0x8014 page in $at on its own, and the successful path reaches the shared
 *   epilogue through its own `j` with the 1 in the delay slot while the first
 *   test's target is the shared zero-return block.
 *
 * Name evidence: 0x801448EB is the shared state selector that game00 dispatches
 *   the handler table 0x801C84BC through (dispatchIndexedStateHandler reads it
 *   as an s8 table index) and that this tree's world overlays set as their state
 *   entry, and 0x801448EC is the shared progress byte the same overlays store
 *   beside it, so the name states the entry/progress pair it writes. No image or
 *   code word of this target or of the main executable holds 0x801F2F58 (no
 *   table entry, no `jal`), so the caller and the two argument roles are
 *   unevidenced and stay structural as packed-word half ranges.
 *
 * Shape: the statement order and operand forms follow the exact sibling
 *   startStateEntry0DProgress0WhenMapRangeAndModeGateWorld02Area09713_801F2C98,
 *   which gates on `(u32)((arg0 >> 16) - K) >= N` with the same `sra`/`addiu`/
 *   `sltiu`/`beqz` sequence, calls a helper and then stores a state entry and a
 *   progress byte; this target's own constants and symbols come from the
 *   original bytes above.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startStateEntry3AProgress0AWhenPackedArgRangesWorld03Area15013_801F2F58(
    s32 arg0, s32 arg1) {
  if ((u32)((arg0 >> 16) - 0x17) >= 6u) {
    return 0;
  }
  if ((u32)((arg1 >> 16) - 0x33) >= 6u) {
    return 0;
  }
  func_8015C088();
  D_801448EB = 0x3A;
  D_801448EC = 0xA;
  return 1;
}
