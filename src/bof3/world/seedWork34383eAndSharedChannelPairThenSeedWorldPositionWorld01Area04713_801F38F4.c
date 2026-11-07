#include "bof3/bof3.h"

/* Scratchpad work cursor cell at 0x1F800044. Bound through the target symbol
 * table so each read expands to the split-address form `lui %hi` plus a folded
 * `%lo` displacement; a literal cell address instead makes one constant-address
 * pseudo that the compiler hoists into a call-saved register across the
 * resident call at 0x801F3924. */
/* @source 0x1F800044 @kind unknown */
extern u8* area04713WorkCursor_1F800044;

/* Shared view channel pair at 0x80149308/0x8014930C. The bmagic siblings add
 * this pair to work words 0x34/0x38 (work[13]/work[14]), so the pair is the
 * base the record coordinate words this function seeds are relative to. */
/* @source 0x80149308 @kind unknown */
extern u32 D_80149308;
/* @source 0x8014930C @kind unknown */
extern u32 D_8014930C;
/* Shared signed world position halfwords at 0x801492D8/0x801492DC. */
/* @source 0x801492D8 @kind unknown */
extern s16 D_801492D8;
/* @source 0x801492DC @kind unknown */
extern s16 D_801492DC;

/* Resident helpers outside this payload; both already have generated
 * undefined_funcs_auto.txt rows, so they stay address-anchored. */
void func_80153DE8(void);
void func_80154FD8(u32 arg0);

/* @source 0x801F38F4
 * @behavior Seeds the view of this overlay through the scratchpad work record
 *           at 0x1F800044: stores 0x120000 in the record coordinate word at
 *           0x34 and 0x240000 in the coordinate word at 0x38 and mirrors the
 *           same two words into the shared channel pair at 0x80149308 and
 *           0x8014930C, runs the resident helper at 0x80153DE8, then stores
 *           0x1220 in the record halfword at 0x3E and passes that same value to
 *           the resident helper at 0x80154FD8 before seeding the shared signed
 *           world coordinates at 0x801492D8 with -0x48A and at 0x801492DC with
 *           0xF8. Takes no arguments and returns nothing.
 *
 * Shape notes from the original bytes. The range opens with its own
 * `addiu $sp, $sp, -0x18` / `sw $ra, 0x10($sp)` frame and closes with
 * `jr $ra` at 0x801F3964, so it is a complete function boundary and not a
 * shared epilogue or fragment. The cell at 0x1F800044 is read twice
 * (0x801F38FC and 0x801F3934): the resident call at 0x801F3924 splits the two
 * groups, so neither read is a redundant one and no callee-saved register holds
 * the pointer - each group keeps the pointer in $v1 only for its own stores.
 * Both reads expand to `lui $v1, %hi` followed by `lw $v1, 0x44($v1)`, so the
 * cell is spelled through its target symbol rather than a literal address: a
 * literal cell address collapses to one constant-address pseudo that the
 * compiler keeps in $16 across that call and then loads with displacement 0,
 * which costs a save and a restore in the frame and the second `lui`.
 * The coordinate-word offsets come from exact siblings: the bmagic lifts add
 * the 0x80149308/0x8014930C pair to work[13] and work[14] (0x34 and 0x38), and
 * the exact area12114 lift stores the shared position words at 0x34/0x38 and
 * the shared halfword at 0x3E of the same scratchpad record. The halfword at
 * 0x3E takes the value the resident helper at 0x80154FD8 is called with, the
 * same commit-then-pass shape the exact SCENA00 lift func_801FB204 uses when it
 * commits its view-scale word and calls that helper with the high halfword.
 * Both constants are stored as signed immediates: -0x48A materialises with a
 * single `addiu $v0, $zero, -0x48A` at 0x801F3944, so the channel is declared
 * signed, as the exact SCENA00 lift func_801FB204 also declares it when it
 * seeds its own value of this channel. The payload script data
 * at 0x801F3E5C and 0x801F3E60 names this address and its neighbour
 * 0x801F396C, so the function is an area event-handler callback; that caller is
 * script data, not a lifted payload call site, so the argument roles stay empty
 * and no companion check applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWork34383eAndSharedChannelPairThenSeedWorldPositionWorld01Area04713_801F38F4(
    void) {
  u8* work;

  work = area04713WorkCursor_1F800044;
  FIELD_REF(u32, work, 0x34) = 0x120000u;
  D_80149308 = 0x120000u;
  FIELD_REF(u32, work, 0x38) = 0x240000u;
  D_8014930C = 0x240000u;
  func_80153DE8();
  work = area04713WorkCursor_1F800044;
  FIELD_REF(u16, work, 0x3E) = 0x1220u;
  func_80154FD8(0x1220u);
  D_801492D8 = -0x48a;
  D_801492DC = 0xf8;
}
