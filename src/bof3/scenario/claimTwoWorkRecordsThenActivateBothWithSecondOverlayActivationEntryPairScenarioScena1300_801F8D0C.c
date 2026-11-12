#include "bof3/bof3.h"

/* Shared 0x98-byte work-record table in main RAM, outside this overlay's image.
 * The generated undefined_syms_auto.txt binds the address. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Scratchpad halfword that publishes the work-record index being activated. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* This overlay's second 16-byte activation-entry pair inside the pointer run at
 * 0x801FBEF0: the two records at 0x801FBF98 and 0x801FBFA8, which follow the
 * pair at 0x801FBF78 and precede the thirteen-word record-callback table at
 * 0x801FBFB8. This body hands both entries to func_801A65A0. */
extern u8 D_801FBF98[]; /* @source 0x801FBF98 @kind data */

/* Shared main-executable work-slot allocator: it hands out free indices of the
 * 0x80146888 record table and reports 0xFF when none is left. This body masks
 * its return with 0xFF at every use, so the return type is byte-width. */
u8 func_8015CA48(void);
void func_801A65A0(u8 *entry);

/* @source 0x801F8D0C
 * @behavior Claims the second pair of shared 0x98-byte work records this overlay
 * activates, the pair its own activation-entry records at 0x801FBF98 and
 * 0x801FBFA8 arm. It asks the shared allocator func_8015CA48 for a free record
 * identifier and returns when that identifier is 0xFF. Otherwise it sets the
 * flag byte of the 0x80146888 record the identifier scales, asks the allocator
 * for a second identifier, and when the second one is 0xFF it clears that first
 * record flag byte again and returns. With two identifiers it loads this
 * overlay's second activation-entry pair at 0x801FBF98, sets the second record
 * flag byte, and for each identifier publishes it as the halfword at 0x1F800000
 * and hands the entry to func_801A65A0, which resolves that halfword into the
 * 0x80146888 table and applies the entry fields to the resolved record. The
 * first activation takes the entry at 0x801FBF98 and the second the entry 0x10
 * bytes past it, so this body differs from its sibling at 0x801F8C1C only in
 * which of the overlay's two entry pairs it arms: this span carries the base
 * 0x801FBF98 where that sibling carries 0x801FBF78. It requests no resource and
 * keeps both identifiers in saved registers instead of publishing them to main
 * RAM. It takes no arguments and returns nothing; its 0x28-byte frame keeps $ra
 * at 0x24 and $s0 to $s4 at 0x10 to 0x20, with $s0 holding the first record
 * offset across the second allocator call and then becoming the 0x1F800000
 * base, $s4 holding the stored flag value 1 across the same call and $s1
 * holding 0xFF for both comparisons before it becomes the entry base.
 *
 * Shape and statement order come from the same target's near-identical sibling
 * claimTwoWorkRecordsThenActivateBothWithOverlayActivationEntriesScenarioScena1300_801F8C1C
 * (0x801F8C1C), whose 240 bytes run the same allocator pair, the same 0x80146888
 * flag stores and the same two func_801A65A0 activations over entries 0x10 bytes
 * apart. That span and this one differ in exactly two words: its internal
 * `j` target, which shifts by the 0xF0 span delta, and the carried entry-base
 * immediate, so the entry base is this body's own constant and the rest of the
 * shape is the sibling's. An own scan of every extracted payload under
 * out/binaries, masking each `jal`/`j`/branch word, found this span at offset
 * 0x210C in this payload alone, so no byte-identical twin exists in the corpus
 * and the clone lever has no representative.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimTwoWorkRecordsThenActivateBothWithSecondOverlayActivationEntryPairScenarioScena1300_801F8D0C(
    void) {
  u8 first;
  u8 second;
  s32 offset;
  u8 *entry;
  u16 *scratch;

  first = func_8015CA48();
  if (first == 0xFF) {
    return;
  }
  offset = first * 152;
  D_80146888[offset] = 1;
  second = func_8015CA48();
  if (second == 0xFF) {
    D_80146888[offset] = 0;
    return;
  }
  entry = D_801FBF98;
  scratch = &D_1F800000;
  D_80146888[second * 152] = 1;
  *scratch = first;
  func_801A65A0(entry);
  *scratch = second;
  func_801A65A0(entry + 0x10);
}
