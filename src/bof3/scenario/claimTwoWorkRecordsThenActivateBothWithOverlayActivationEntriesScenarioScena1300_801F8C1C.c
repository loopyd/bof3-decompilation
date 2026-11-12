#include "bof3/bof3.h"

/* Shared 0x98-byte work-record table in main RAM, outside this overlay's image.
 * The generated undefined_syms_auto.txt binds the address. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Scratchpad halfword that publishes the work-record index being activated. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* This overlay's own activation entries: the four 16-byte records that follow
 * the twelve-word state-handler run at 0x801FBF48 and precede the thirteen-word
 * record-callback table at 0x801FBFB8. This body hands the first two of them to
 * func_801A65A0. */
extern u8 D_801FBF78[]; /* @source 0x801FBF78 @kind data */

/* Shared main-executable work-slot allocator: it hands out free indices of the
 * 0x80146888 record table and reports 0xFF when none is left. This body masks
 * its return with 0xFF at every use, so the return type is byte-width. */
u8 func_8015CA48(void);
void func_801A65A0(u8 *entry);

/* @source 0x801F8C1C
 * @behavior Claims the pair of shared 0x98-byte work records this overlay
 * activates through its own activation entries. It asks func_8015CA48 for a
 * free record identifier and returns when that identifier is 0xFF. Otherwise it
 * sets the flag byte of the 0x80146888 record the identifier indexes, asks the
 * allocator for a second identifier, and when the second one is 0xFF it clears
 * that first record flag byte again and returns. With two identifiers it loads
 * this overlay's activation entries at 0x801FBF78, sets the second record flag
 * byte, and for each identifier publishes it as the halfword at 0x1F800000 and
 * hands the entry to func_801A65A0, which resolves that halfword into the
 * 0x80146888 table and applies the entry fields to the resolved record. The
 * first activation takes the entry at 0x801FBF78 and the second the one 0x10
 * bytes past it. Unlike the scena03 and scena08 siblings this body requests no
 * resource and keeps both identifiers in saved registers instead of publishing
 * them to main RAM. The second record index is written inline at its store
 * rather than through the first record's offset local: that spelling measured
 * 60 of 60 instructions and 240 of 240 bytes here, where a reassignment of the
 * same local measured 53 of 60 with the entry-base materialisation and the
 * 0x1F800000 base load scheduled around the scaled index. It takes no
 * arguments and returns nothing; its 0x28-byte frame keeps $ra at 0x24 and $s0
 * to $s4 at 0x10 to 0x20, with $s0 holding the first record offset across the
 * second allocator call and then becoming the 0x1F800000 base, $s4 holding the
 * stored flag value 1 across the same call and $s1 holding 0xFF for both
 * comparisons before it becomes the entry base.
 *
 * Statement order and operand forms come from the near-identical sibling
 * claimTwoEffectSlotsThenActivateBothWithTemplatesScenarioScena0800_801FD07C
 * (0x801FD07C), which runs the same allocator pair, the same 0x80146888 flag
 * stores and the same two func_801A65A0 activations with entries 0x10 bytes
 * apart. This span differs by masking both identifiers with 0xFF at every use,
 * by keeping them and the first record offset in saved registers rather than
 * reloading slots it published, and by using its own entry base and its own
 * pair of record references; one payload scan over out/binaries found these 240
 * bytes at offset 0x201C in this payload alone, so no byte-identical twin
 * exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimTwoWorkRecordsThenActivateBothWithOverlayActivationEntriesScenarioScena1300_801F8C1C(
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
  entry = D_801FBF78;
  scratch = &D_1F800000;
  D_80146888[second * 152] = 1;
  *scratch = first;
  func_801A65A0(entry);
  *scratch = second;
  func_801A65A0(entry + 0x10);
}
