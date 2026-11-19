#include "bof3/bof3.h"

extern s8 D_801490C7;    /* @source 0x801490C7 @kind data */
extern u16 D_801490A8;   /* @source 0x801490A8 @kind data */
extern u16 D_801F5CD4[]; /* @source 0x801F5CD4 @kind table */

/* Shared scenario flag word of the main executable. */
extern u32 D_8014686C;

/* @source 0x8015B580 - shared block bit-set helper (owner, bit). */
extern void func_8015B580(u32 arg0, u8 bit_index);

/* @source 0x801F2C04
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F5CD4 and
 *           publishes it into the shared status halfword D_801490A8; when that
 *           same mode reads 0 it then sets flag bit 0x3B of the shared scenario
 *           flag word D_8014686C through the shared bit-set helper
 *           func_8015B580, and every other mode leaves the flag word alone.
 *           Takes no arguments and returns nothing; its 0x18-byte frame only
 *           keeps $ra across that call.
 *
 *           Constant and store evidence from these original bytes: the mode
 *           cell 0x801490C7 is read once with `lui`/`lb` (signed byte) before
 *           the frame reservation, the table base 0x801F5CD4 is indexed by
 *           `sll` by one and `addu` into $at with `lhu`, the status cell
 *           0x801490A8 takes the `sh`, the skip test is the single `bnez` on
 *           the mode register with a delay `nop`, and the flag path reads the
 *           word at 0x8014686C with `lui`/`lw` into $a0 before
 *           `jal func_8015B580` with 0x3B in $a1. The mode value stays live
 *           across the table publish, so the index and the halfword share $v1
 *           while $v0 keeps the mode; the leaf sibling at 0x801F2C54, whose
 *           mode dies at the store, reuses $v0 for both.
 *
 *           Shape: the 0x50-byte span has no byte-identical sibling anywhere
 *           in out/binaries (a scan of all 398 payloads finds these 80 bytes
 *           only at their own offset 4), so this is a clean-C seed rather than
 *           a clone. Statement order, the single `mode` temporary and the one
 *           `if` arm follow the exact same-target sibling
 *           publishModeStatusFromTable5cd8World01Area04914_801F2C54
 *           (0x801F2C54), which publishes the neighbouring halfword table
 *           0x801F5CD8 through the same mode and status cells, and the exact
 *           corpus sibling publishModeStatusFromTable5354AndSetFlagBit15
 *           World02Area09413_801F2C5C, which publishes its own table through
 *           those same cells and then calls func_8015B580(D_8014686C, 0x15)
 *           from a second compare; this span carries a single `bnez` on the
 *           mode instead of that two-case dispatch, which is what an `if` on
 *           mode 0 leaves behind, so only one arm is written. The table
 *           literal 0x801F5CD4 and the bit 0x3B are this payload's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable5cd4AndSetFlagBit3BWhenModeZeroWorld01Area04914_801F2C04(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F5CD4[mode];
  if (mode == 0) {
    func_8015B580(D_8014686C, 0x3B);
  }
}
