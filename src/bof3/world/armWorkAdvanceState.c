#include "bof3/bof3.h"

/* Scratchpad work cursor cell at 0x1F800044, the pointer cell this overlay
 * publishes its work record through. The target symbol table binds it, so the
 * read expands to the split-address form `lui %hi` plus a folded `%lo`
 * displacement; the same cell is declared in
 * seedWork34383eAndSharedChannelPairThenSeedWorldPositionWorld01Area04713_801F38F4.c. */
/* @source 0x1F800044 @kind unknown */
extern u8* area04713WorkCursor_1F800044;

/* @source 0x801F2C48
 * @behavior Overlay step handler at index 0 of the local handler table based at
 *           0x801F39C8 (this target trailing textbin, whose first four words are
 *           the four step handlers of the run): reads the state byte at +1 of
 *           the work record published by the scratchpad pointer cell
 *           0x1F800044, stores 0x3C into the record 32-bit countdown word at
 *           +0xC and writes the state byte back incremented by one, so the next
 *           dispatch of callStateTable39World01
 *           enters the following handler. It takes no arguments, returns
 *           nothing and uses no frame.
 *
 * Shape notes from the original bytes: the 0x24-byte body loads the state byte
 * before the countdown store (`lbu $v1, 0x1($v0)` at 0x801F2C54, `sw $a0,
 * 0xC($v0)` at 0x801F2C5C) and writes the incremented byte in the `jr $ra`
 * delay slot, so the source reads the byte into a local first and assigns the
 * bumped value after the store. The countdown word is the one the next handler
 * tickCountdownClearTable
 * decrements and reloads with 0x230 when it reaches zero, and the
 * state byte is the one the dispatcher scales by four into its handler table, so
 * this body arms the first 0x3C-step countdown of the run and advances the run
 * to that next handler. No other extracted payload holds this body: a payload
 * scan over out/binaries found the 36 bytes only at 0x48 of this target, so no
 * byte-identical sibling seeded it; the store-then-increment shapes of
 * clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0700_801F6E54 and
 * seedSampleRingAt800E5C00ClearWorkTableByte25ArmCountdown512AndAdvanceWorkByte1ScenarioScena0400_801F77EC
 * supplied only the countdown-then-advance vocabulary, not this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armWorkAdvanceState(void) {
  u8* work;
  u8 state;

  work = area04713WorkCursor_1F800044;
  state = FIELD_REF(u8, work, 0x1);
  FIELD_REF(u32, work, 0xC) = 0x3c;
  FIELD_REF(u8, work, 0x1) = state + 1;
}
