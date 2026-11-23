#include "bof3/bof3.h"

/* Scratchpad work cursor cell at 0x1F800044, the pointer cell this overlay
 * publishes its work record through. Bound through the target symbol table so
 * the read expands to the split-address form `lui %hi` plus a folded `%lo`
 * displacement; the same cell is declared in
 * armWorkAdvanceState.c and
 * seedWork34383eAndSharedChannelPairThenSeedWorldPositionWorld01Area04713_801F38F4.c. */
/* @source 0x1F800044 @kind unknown */
extern u8* area04713WorkCursor_1F800044;

/* Overlay draw step of the same work record, still on its `asm` row at
 * 0x801F34A0. It loads the record words 0x34, 0x38 and 0x3C into the RotTrans
 * argument triple (0x801F354C..0x801F356C) and installs the projected matrix,
 * so it takes no arguments and returns nothing. */
void func_801F34A0(void);

/* @source 0x801F3448
 * @behavior Overlay work-record seeding step: reads the work record published
 *           by the scratchpad pointer cell 0x1F800044 once and tests its
 *           unsigned byte at +0x02. While that byte reads zero it seeds the
 *           record world position words with -0x3900 at +0x34, -0x3880 at
 *           +0x38 and -0x7F0 at +0x3C, then re-reads the very same byte and
 *           writes it back incremented by one, so a later pass of this handler
 *           no longer reseeds. It then always runs the overlay draw step at
 *           0x801F34A0, which loads those three words and projects them. Takes
 *           no arguments and returns nothing.
 *
 * Shape notes from the original bytes. The range opens with its own
 * `addiu $sp, $sp, -0x18` / `sw $ra, 0x10($sp)` frame at 0x801F3450 and closes
 * with `jr $ra` at 0x801F3498, so it is a complete 0x58-byte function boundary
 * and neither a shared epilogue nor a fragment. The cell read at 0x801F3448
 * has been scheduled above that frame setup, and the `lui` pair with the folded
 * 0x44 displacement is the target-symbol form of the cell rather than a literal
 * address.
 * The byte at +0x02 is read twice: the test at 0x801F3458 and again at
 * 0x801F3474, after the +0x34 and +0x38 stores and before the +0x3C store, so
 * the body reads the byte a second time instead of carrying the tested value
 * across the stores. All three constants materialise as a single `addiu`
 * immediate (0xC700, 0xC780, 0xF810), the sign-extended 16-bit form of
 * 0xFFFFC700, 0xFFFFC780 and 0xFFFFF810.
 * The word roles come from exact lifts of the same record: this target's
 * seedWork34383eAndSharedChannelPairThenSeedWorldPositionWorld01Area04713_801F38F4
 * seeds 0x34 and 0x38 as the coordinate words it also mirrors into the shared
 * channel pair at 0x80149308 and 0x8014930C, and
 * drawWorkQuadAtFixedOffsetWorld02Area10013_801F31A0 and
 * passWorkPositionPairAndDistanceToQuadEmitterWorld01Area05913_801F3060 pass
 * record word 0x3C as the third world coordinate.
 * No word of this 0x12A8-byte payload names 0x801F3448 or 0x801F34A0: the step
 * table at 0x801F39C8 holds 0x801F2C48, 0x801F2C6C, 0x801F2CC8 and 0x801F2D44,
 * and the script blob at 0x801F3E5C names only 0x801F38F4 and 0x801F396C, so
 * the caller lies outside the payload and the argument roles stay structural.
 * A payload scan over out/binaries for these 0x58 bytes found them only at file
 * offset 0x848 of this target, so no byte-identical sibling seeded the body and
 * the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePositionClearDraw(
    void) {
  u8* work;

  work = area04713WorkCursor_1F800044;
  if (FIELD_REF(u8, work, 0x2) == 0) {
    FIELD_REF(u32, work, 0x34) = 0xFFFFC700;
    FIELD_REF(u32, work, 0x38) = 0xFFFFC780;
    FIELD_REF(u32, work, 0x3C) = 0xFFFFF810;
    FIELD_REF(u8, work, 0x2) = FIELD_REF(u8, work, 0x2) + 1;
  }
  func_801F34A0();
}
