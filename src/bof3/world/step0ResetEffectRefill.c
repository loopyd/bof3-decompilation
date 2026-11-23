#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The original materialises the cell
 * address with `lui` plus an offset load once for the mode-byte store and the
 * countdown store, and again for the step advance; the two loads land in
 * different registers, so the source keeps one pointer local per group. The
 * cell sits outside every overlay payload, this target's own symbol map binds
 * nothing at the address, and the `D_` spelling auto-binds.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Reset routine of this target at 0x801F35B8, still on its asm row. It stores
 * 0x250000 into work record word +0x34, 0x748000 into word +0x38 and the
 * left-shifted return of func_8015477C into word +0x3C, then clears the lead
 * byte of all 256 entries of the 0x1C-byte-stride effect table at 0x800E4800.
 * It takes no arguments and returns nothing, so this handler reads no result. */
extern void func_801F35B8(void); /* @source 0x801F35B8 */

/* Same-binary cue dispatcher at 0x8015DF18, which the generated
 * undefined_funcs_auto file of this target binds for comparison. */
extern void func_8015DF18(u32 cue);

/* @source 0x801F2C5C
 * @behavior Step 0 of the run this overlay dispatches through the handler
 *           table at 0x801F43C0: the dispatcher at 0x801F2C18 indexes that
 *           table with work-record byte 0x01, and entry 0 of the table holds
 *           this address. The body first runs the target-local reset routine
 *           at 0x801F35B8 and ignores its result, which seeds the work record
 *           words +0x34, +0x38 and +0x3C and clears the lead byte of the 256
 *           effect-table entries. It then stores 0 into work record byte +0x06,
 *           the byte the per-frame sweep at 0x801F36B0 reads as the state step
 *           it adds. It reads the pointer cell a second time and refills the
 *           record unsigned halfword at +0x32 with 0x78, the countdown the
 *           handler at 0x801F2CB4 decrements, through the pointer of the first
 *           read, then advances the record byte at +0x01 through the pointer of
 *           the second read. Byte 0x01 is the byte the table dispatcher
 *           indexes, so the increment sends the next frame to entry 1, the
 *           handler at 0x801F2CB4. It closes by queueing cue 0x201 through the
 *           same-binary cue dispatcher func_8015DF18. Takes no arguments,
 *           returns nothing, and its 0x18-byte frame holds only the saved
 *           return address at 0x10. The roles of the reset words, of the
 *           countdown and of the cue stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void step0ResetEffectRefill(
    void) {
  u8* work;
  u8* stage;

  func_801F35B8();
  work = D_1F800044;
  work[6] = 0;
  stage = D_1F800044;
  *(u16*)(work + 0x32) = 0x78;
  stage[1]++;
  func_8015DF18(0x201);
}
