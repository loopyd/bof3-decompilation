#include "bof3/bof3.h"

/* Scratchpad work cursor cell at 0x1F800044, the pointer cell this overlay
 * publishes its work record through. The target symbol table binds it, so the
 * read expands to the split-address form `lui %hi` plus a folded `%lo`
 * displacement; the same cell is declared in
 * armWorkAdvanceState.c,
 * tickCountdownClearTable.c
 * and stateChannelPairWorld.c. */
/* @source 0x1F800044 @kind unknown */
extern u8* scratchWorkCursor;

/* Shared main-RAM frame word at 0x80143E6C, bound by
 * config/targets/shared/symbols.txt for every target that composes it, so this
 * overlay needs no local map row. Only its low three bits are read here. */
/* @source 0x80143E6C @kind unknown */
extern u32 D_80143E6C;

/* Effect step of this payload at 0x801F31F4: draws the overlay's soft radial
 * disc for the signed halfword extent it is handed, and its result is unused.
 * The exact lift lives in
 * renderPositionGouraudDisc.c and the target map
 * binds the name at 0x801F31F4. */
void renderPositionGouraudDisc(s16 arg0);

/* Per-frame work-table step of this payload at 0x801F330C: arms, advances and
 * retires the records of the shared 0x18-byte-stride table at 0x800E4800 and
 * returns its low byte set when any record was active. The call site here
 * ignores that result, which agrees with the call form of the exact lift in
 * stateRecordsReportActive.c. */
u8 stateRecordsReportActive(void);

/* Claim step of this payload at 0x801F32C4: marks the first free record of the
 * same shared work table active and clears its sub-phase byte. It takes no
 * arguments and its result is unused. The exact lift lives in
 * claimFreeSubPhase.c and
 * the target map binds the name at 0x801F32C4. */
void claimFreeSubPhase(
    void);

/* @source 0x801F2CC8
 * @behavior Overlay step handler at index 2 of the local four-word handler run
 *           based at 0x801F39C8 (this target trailing textbin). It first draws
 *           this overlay's soft radial disc at the fixed extent 0x80, then runs
 *           the per-frame step over the shared work table at 0x800E4800 and
 *           discards the activity report. While the shared frame word
 *           D_80143E6C reads a multiple of 8, i.e. on the fixed spawn period,
 *           it claims the next free work record of that table. It then reads the
 *           32-bit countdown word at +0xC of the record published by the
 *           scratchpad pointer cell 0x1F800044, stores it back decremented by
 *           one and tests that decremented value; while it reads zero the
 *           handler reloads the same word with 0x10 and writes the record state
 *           byte at +0x1 back incremented by one, so the dispatcher
 *           dispatchByte1Table39c8 enters the
 *           following handler of the run on its next dispatch. It takes no
 *           arguments, returns nothing and reserves a 0x18-byte frame that
 *           holds only the return address.
 *
 * Shape notes from the original bytes. The range opens with its own `addiu $sp,
 * $sp, -0x18` / `sw $ra, 0x10($sp)` frame at 0x801F2CC8 and closes with `jr
 * $ra` at 0x801F2D3C, so it is a complete 0x7C-byte function boundary and
 * neither a shared epilogue nor a fragment. The two leading calls keep their
 * `nop` delay slots and the constant 0x80 arrives as `addiu $a0, $zero, 0x80`
 * in the first `jal` slot at 0x801F2CD4. The frame-word test materialises with
 * `lui %hi(D_80143E6C)` / `lw %lo` at 0x801F2CE0 and `andi $v0, $v0, 0x7` at
 * 0x801F2CEC, and its `bnez` at 0x801F2CF0 skips the claim call, so the test is
 * an equality on the masked word rather than a bounded comparison. The
 * scratchpad cell is read once at 0x801F2D00, after the claim call, so the
 * countdown and the state-byte group share that one pointer. The decremented
 * value is carried in $v0 from `addiu $v0, $v0, -1` at 0x801F2D14 to the `bnez`
 * at 0x801F2D18 and the store that swallows that branch delay slot, so the
 * source tests the decremented value instead of re-reading the word. The reload
 * materialises 0x10 as a single `addiu $v1, $zero, 0x10` at 0x801F2D24 and its
 * store at 0x801F2D28 sits between the `lbu` of the state byte at 0x801F2D20
 * and the `sb` of the incremented value at 0x801F2D30, which is why the byte is
 * held in a local across that store. The `nop` words at 0x801F2CE8, 0x801F2D08
 * and 0x801F2D10 are the load-delay slots of the three loads.
 * The statement order, operand forms and offset spelling come from the exact
 * siblings of this run: the frame and the two leading calls from
 * renderPositionGouraudDisc and
 * stateRecordsReportActive, the
 * decrement-then-test form `--FIELD_REF(u32, work, 0xC) == 0` and the
 * reload-then-advance tail from
 * tickCountdownClearTable,
 * and the masked-word guard from the same frame word's use in
 * claimFreeSubPhase.
 * No word of this 0x12A8-byte payload calls 0x801F2CC8: it is reached only as
 * entry 2 of the four-word pointer run at 0x801F39C8, so it is a dispatch target
 * and its argument roles stay structural. A payload scan over every file under
 * out/binaries found these 0x7C bytes only at file offset 0xC8 of this target,
 * so no byte-identical sibling seeded the body and the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderDiscSpawnPeriod(
    void) {
  u8 state;
  u8* work;

  renderPositionGouraudDisc(0x80);
  stateRecordsReportActive();
  if ((D_80143E6C & 7) == 0) {
    claimFreeSubPhase();
  }
  work = scratchWorkCursor;
  if (--FIELD_REF(u32, work, 0xC) == 0) {
    state = FIELD_REF(u8, work, 0x1);
    FIELD_REF(u32, work, 0xC) = 0x10;
    FIELD_REF(u8, work, 0x1) = state + 1;
  }
}
