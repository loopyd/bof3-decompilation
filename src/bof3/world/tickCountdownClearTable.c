#include "bof3/bof3.h"

/* Scratchpad work cursor cell at 0x1F800044, the pointer cell this overlay
 * publishes its work record through. The target symbol table binds it, so the
 * read expands to the split-address form `lui %hi` plus a folded `%lo`
 * displacement; the same cell is declared in
 * armWorkAdvanceState.c and
 * stateChannelPairWorld.c. */
/* @source 0x1F800044 @kind unknown */
extern u8* scratchWorkCursor;

/* Release step of the shared 0x18-byte-stride work table at 0x800E4800: it
 * zeroes byte 0x14 of each of its 0x10 records, takes no arguments and returns
 * nothing, so the call site here matches its no-argument void prototype. The
 * exact lift lives in stateWorkTableByte14.c
 * and the target map binds the name at 0x801F3298. */
void stateWorkTableByte14(void);

/* @source 0x801F2C6C
 * @behavior Overlay step handler at index 1 of the local handler table based at
 *           0x801F39C8 (this target trailing textbin, whose first four words are
 *           the four step handlers of the run): reads the 32-bit work countdown
 *           word at +0xC of the record published by the scratchpad pointer cell
 *           0x1F800044, stores it back decremented by one and tests that
 *           decremented value. While it reads zero the handler reloads the same
 *           word with 0x230, calls
 *           stateWorkTableByte14 to release
 *           the shared work table at 0x800E4800, then re-reads the scratchpad
 *           cell and writes the record state byte at +0x1 back incremented by
 *           one, so the dispatcher
 *           dispatchByte1Table39c8 enters the next
 *           handler of the run on its next dispatch. It takes no arguments,
 *           returns nothing and reserves a 0x18-byte frame that holds only the
 *           return address.
 *
 * Shape notes from the original bytes. The range opens with its own `addiu $sp,
 * $sp, -0x18` / `sw $ra, 0x10($sp)` frame at 0x801F2C74 and closes with `jr
 * $ra` at 0x801F2CC0, so it is a complete 0x5C-byte function boundary and
 * neither a shared epilogue nor a fragment. The cell read at 0x801F2C6C is
 * scheduled above the frame setup, and its `lui`/`lw` pair with the folded 0x44
 * displacement is the target-symbol form of the cell rather than a literal
 * address. The decremented value is carried in $v0 from `addiu $v0, $v0, -1` at
 * 0x801F2C84 to the `bnez` at 0x801F2C88 and the store that swallows that
 * branch delay slot, so the source tests the decremented value instead of
 * re-reading the word. The reload at 0x801F2C90 materialises 0x230 as a single
 * `addiu $v0, $zero, 0x230` and its store fills the `jal` delay slot. The
 * scratchpad cell is read a second time at 0x801F2C9C after the call, so the
 * body re-reads the cell for the state-byte group instead of keeping the first
 * pointer in a saved register, and the incremented byte at +0x1 is written back
 * through that second read; the two `nop` words at 0x801F2C80, 0x801F2CA4 and
 * 0x801F2CAC are the load-delay slots of the three loads.
 * The store-then-advance shapes of this target's exact
 * armWorkAdvanceState and
 * statePositionClearDraw
 * supplied the statement order, operand forms and offset spelling, and the
 * decrement-then-test form `--FIELD_REF(...) == 0` comes from the exact
 * stateRecordsReportActive.
 * No word of this 0x12A8-byte payload calls 0x801F2C6C: it is reached only as
 * entry 1 of the four-word pointer run at 0x801F39C8, so it is a dispatch
 * target and its argument roles stay structural. A scan of every payload under
 * out/binaries found these 0x5C bytes only at file offset 0x6C of this target,
 * so no byte-identical sibling seeded the body and the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownClearTable(
    void) {
  u8* work;

  work = scratchWorkCursor;
  if (--FIELD_REF(u32, work, 0xC) == 0) {
    FIELD_REF(u32, work, 0xC) = 0x230;
    stateWorkTableByte14();
    work = scratchWorkCursor;
    FIELD_REF(u8, work, 0x1) = FIELD_REF(u8, work, 0x1) + 1;
  }
}
