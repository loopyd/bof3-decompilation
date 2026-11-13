#include "bof3/bof3.h"

/* Shared 0x98-byte work-record table in main RAM, outside this overlay's image.
 * The generated undefined_syms_auto.txt binds the address. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* Scratchpad halfword that publishes the work-record index being activated. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* This overlay's own 16-byte activation entry in the payload's leading data
 * run, at payload offset 0x21FC. This body hands its address to func_801A65A0,
 * which resolves the published halfword into the 0x80146888 table and applies
 * the entry fields to that record. No other body in this payload reads it. */
extern u8 D_801F4DFC[]; /* @source 0x801F4DFC @kind data */

/* Shared main-executable work-slot allocator: it hands out free indices of the
 * 0x80146888 record table and reports 0xFF when none is left. This body masks
 * its return with 0xFF at every use, so the return type is byte-width. */
u8 func_8015CA48(void);
void func_801A65A0(u8 *entry);
void func_8014D6DC(s32 resource, s32 mode);

/* @source 0x801F4508
 * @behavior Claims one shared 0x98-byte work record for this overlay and
 * activates it. It asks func_8015CA48 for a free record identifier and returns
 * when the masked identifier is 0xFF. Otherwise it sets the flag byte of the
 * 0x80146888 record that the identifier indexes, where the 152-byte stride is
 * built from the masked identifier, publishes that same masked identifier as
 * the halfword at 0x1F800000, and hands this overlay's 16-byte activation entry
 * at 0x801F4DFC to func_801A65A0, which resolves the published halfword into
 * the 0x80146888 table and applies the entry fields to that record. It then
 * requests resource 3 with mode 0 through func_8014D6DC. No jal in this payload
 * and no word of it names this address, so no caller is known and the body
 * stays structural: it takes no arguments and returns nothing. Its 0x18-byte
 * frame holds only $ra at 0x10, the allocator's raw return stays in $a1 and is
 * masked with 0xFF once for the guard and the stride and once more for the
 * scratchpad store, and the entry base is materialised into $a0 ahead of the
 * stride computation while the 0x1F800000 base follows the flag store.
 *
 * Statement order and operand forms come from the near-identical sibling
 * claimTwoWorkRecordsThenActivateBothWithOverlayActivationEntriesScenarioScena1300_801F8C1C
 * (0x801F8C1C), which runs the same allocator, the same 0x80146888 flag store
 * and the same masked-identifier scratchpad publication before handing an entry
 * of its own to func_801A65A0. This span claims a single record, carries one
 * entry and adds the resource request. One payload scan over out/binaries found
 * these 124 bytes at offset 0x1908 in this payload alone, so no byte-identical
 * twin exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkRecordSetFlagPublishIndexActivateEntry4dfcAndRequestResource3World03Area14113_801F4508(
    void) {
  u8 slot;
  s32 offset;
  u8 *entry;
  u16 *scratch;

  slot = func_8015CA48();
  if (slot == 0xFF) {
    return;
  }
  entry = D_801F4DFC;
  offset = slot * 152;
  D_80146888[offset] = 1;
  scratch = &D_1F800000;
  *scratch = slot;
  func_801A65A0(entry);
  func_8014D6DC(3, 0);
}
