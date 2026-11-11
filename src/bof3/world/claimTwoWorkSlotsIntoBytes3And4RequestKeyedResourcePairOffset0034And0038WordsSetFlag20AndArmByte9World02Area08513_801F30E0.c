#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. It is the named cell symbol the exact target lifts of this family
 * declare (setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC,
 * setFlag40UntilWorkByte9ExpiresThenClearSelectedRecordsWorld02Area08513_801F35A0):
 * the original issues its own lui/lw pair for every load, which is what a named
 * cell keeps. The header note in bof3/emi/dispatchers.h records the same
 * load form for the dispatchers that index this cell. */
extern u8* D_1F800044;

/* Scratchpad halfword cell at the base of the scratchpad. The original
 * materialises 0x1F800000 once into a saved register and stores through that
 * base twice, so the cell is declared at halfword width and reached through a
 * pointer local. The scenario lift
 * claimTwoEffectSlotsThenActivateBothWithTemplatesScenarioScena0800_801FD07C
 * declares the same cell the same way. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Work-record flag and row cells of the 0x98-byte-stride table in main RAM,
 * outside this overlay's image. The exact target lifts of this family reach the
 * same table through the base 0x80146888 and the row-offset bases declared
 * here: 0x34, 0x38 and 0x3C are the word triplet clearFlag40...801F37C0 copies
 * from the 0x140-byte records, and 0x5C is the byte this handler clears. Each
 * field keeps its own address-derived base because the original rebuilds
 * lui at,%hi(<base>) and adds the scaled row offset for every access. Splat
 * binds these addresses through the generated undefined_syms_auto.txt rows the
 * remaining asm keeps, so no target-map row is added. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */
extern u8 D_801468BC[]; /* @source 0x801468BC @kind data */
extern u8 D_801468C0[]; /* @source 0x801468C0 @kind data */
extern u8 D_801468C4[]; /* @source 0x801468C4 @kind data */
extern u8 D_801468E4[]; /* @source 0x801468E4 @kind data */

/* Byte 0x4B of the record this handler selects by work byte 0xB: the default
 * record at 0x80145E90 (0x80145E90 + 0x4B) and the record at 0x801469B8
 * (0x801469B8 + 0x4B), the same two records
 * setFlag40UntilWorkByte9ExpiresThenClearSelectedRecordsWorld02Area08513_801F35A0
 * gates bit 0x40 on. They are keyed against the five-row table at 0x801F3D70.
 * What the key selects is not evidenced, so both cells stay address-anchored. */
extern u8 D_80145EDB; /* @source 0x80145EDB @kind data */
extern u8 D_80146A03; /* @source 0x80146A03 @kind data */

/* Word cell at 0x80145ECC -- word 0x3C of the first 0x140-byte record based at
 * 0x80145E90, the template word clearFlag40...801F37C0 copies into a record's
 * 0x3C. This body copies that one word into offset 0x3C of both selected
 * 0x98-byte records, so it is declared at word width here. */
extern s32 D_80145ECC; /* @source 0x80145ECC @kind data */

/* Read-only tables inside this payload, in the textbin region labelled
 * D_801F3D2C. 0x801F3D4C is the base of a run of 0x11-byte rows this handler
 * hands to func_801A4D54 at the row work byte 0xB selects; 0x801F3D70 to
 * 0x801F3D72 are the three byte columns of a five-row keyed table (key, first
 * argument, second argument) searched here. The intervening 0x14-byte and
 * 0xC-byte handler-pointer runs are claimed by
 * src/bof3/world/area08513DispatchTables.c. All four addresses are reached
 * through the generated undefined_syms_auto.txt rows. */
extern u8 D_801F3D4C[]; /* @source 0x801F3D4C @kind data */
extern u8 D_801F3D70[]; /* @source 0x801F3D70 @kind data */
extern u8 D_801F3D71[]; /* @source 0x801F3D71 @kind data */
extern u8 D_801F3D72[]; /* @source 0x801F3D72 @kind data */

/* Shared main-executable services, all outside this overlay's image.
 *
 * func_8015CA48 is the slot allocator the scenario lifts call twice in a row:
 * claimTwoEffectSlotsThenActivateBothWithTemplatesScenarioScena0800_801FD07C
 * and claimWorkSlotFromIndexedWorkRecordPositionScenarioScena0300_801FC1D0
 * record that it hands out identifiers in 0x00..0xFF and returns 0xFF when no
 * slot is left, which is the value this body tests twice.
 *
 * func_801A4D54 takes one 0x11-byte row of the table at 0x801F3D4C.
 *
 * func_8014D6DC takes the resource id and mode pair the scenario lift above
 * passes as (0x4C, 0) / (0x36, 0); here both arguments come from the searched
 * table row.
 *
 * func_8019651C takes the selected record plus four scalar arguments, the last
 * one on the stack; this body passes the 0xF and the 1 in two different
 * argument positions for its two records. The roles of those scalars are not
 * evidenced, so the prototype keeps positional names. */
s32 func_8015CA48(void); /* @source 0x8015CA48 */
void func_801A4D54(u8* entry); /* @source 0x801A4D54 */
void func_8014D6DC(s32 resource, s32 mode); /* @source 0x8014D6DC */
void func_8019651C(u8* record, s32 arg1, s32 arg2, s32 arg3, s32 arg4); /* @source 0x8019651C */

/* @source 0x801F30E0
 * @behavior Work-byte-1 step handler, entry 0 of the local handler table at
 *           0x801F3D40. It asks the shared slot allocator func_8015CA48 for two
 *           slot identifiers and stores them into work bytes 3 and 4 of the
 *           work record published at the scratchpad pointer cell 0x1F800044;
 *           each identifier that is not 0xFF has the flag byte of its
 *           0x98-byte record in the table at 0x80146888 set to 1, and an 0xFF
 *           answer returns at once (after clearing the already-claimed first
 *           record's flag byte in the second case). With both slots it selects
 *           one byte-0x4B key cell by work byte 0xB -- 0x80145EDB when that
 *           byte is nonzero, 0x80146A03 when it is zero -- and searches the
 *           five-row key table at 0x801F3D70 for that key. It then publishes
 *           each slot identifier as the halfword at 0x1F800000, hands the
 *           0x11-byte row at 0x801F3D4C that work byte 0xB indexes to
 *           func_801A4D54 and requests the resource and mode the searched table
 *           row carries through func_8014D6DC, once per slot, republishing the
 *           incoming record pointer afterwards. For the slot in work byte 3 it
 *           lowers the record word at 0x34 by 0x1000 and raises the word at 0x38
 *           by 0x1000; for the slot in work byte 4 it raises 0x34 and lowers
 *           0x38 by the same amount. It copies the shared word at 0x80145ECC
 *           into offset 0x3C of both records, sets bit 0x20 of both flag bytes,
 *           clears byte 0x5C of both, runs func_8019651C on both with the 0xF
 *           and the stack argument 1 split between the second and fourth
 *           arguments, and finally arms work byte 9 to 4 and switches work byte
 *           1 to 1, the countdown and next table index
 *           setFlag40UntilWorkByte9ExpiresThenClearSelectedRecords...801F35A0
 *           consumes. Takes no arguments and returns nothing.
 *
 *           Shape facts from the original bytes: every work-byte and record-byte
 *           access after a store goes back through the published pointer cell,
 *           so the cell is re-read once per access group; the record pointer is
 *           held in $s3; the two slot identifiers are read back through the cell
 *           rather than reused from the allocator's return; the 0xFF constant is
 *           hoisted to $s1 for both range tests; the search counter is a 32-bit
 *           local masked to a byte for the 3-byte row stride and the
 *           unsigned-compared bound 5; the 0x1F800000 base and the 0x80146888
 *           base are each materialised once into a saved register; and the two
 *           func_8019651C calls share that base register across the first call.
 * @status partial
 * @match 93.77
 * @residual The current build matches 286 of 305 compared instructions;
 *           the original is 1216 bytes and this build is 1220 bytes. The frame
 *           and saved-register offsets agree. The search counter survives the
 *           first table-row call in $s0, then becomes the three-byte row offset.
 *           Remaining differences include key-table base materialisation and
 *           comparison operand order, and scheduling around the slot halfword,
 *           copied words, flag bytes and cleared bytes. The scalar template
 *           word is loaded directly from 0x80145ECC, without dereferencing its
 *           value.
 */
void claimTwoWorkSlotsIntoBytes3And4RequestKeyedResourcePairOffset0034And0038WordsSetFlag20AndArmByte9World02Area08513_801F30E0(
    void) {
  /* The original reserves 16 unreferenced frame bytes. This declaration
   * reconstructs their extent, not a recovered local type or purpose. */
  u8 unusedFrame[16];
  u32 offset;
  u32 row;
  s32 accum;
  s32 word;
  u8 value;
  u8* cursor;
  u8* pair;
  u16* slot;
  u8 key;
  u32 i;

  D_1F800044[3] = func_8015CA48();
  if (D_1F800044[3] == 0xFF) {
    return;
  }
  offset = D_1F800044[3] * 0x98;
  D_80146888[offset] = 1;
  D_1F800044[4] = func_8015CA48();
  if (D_1F800044[4] == 0xFF) {
    offset = D_1F800044[3] * 0x98;
    D_80146888[offset] = 0;
    return;
  }
  offset = D_1F800044[4] * 0x98;
  D_80146888[offset] = 1;

  cursor = D_1F800044;
  if (cursor[0xB] != 0) {
    key = D_80145EDB;
  } else {
    key = D_80146A03;
  }
  for (i = 0; (u8)i < 5; i++) {
    if (D_801F3D70[(u8)i * 3] == key) {
      break;
    }
  }
  slot = &D_1F800000;

  *slot = cursor[3];
  func_801A4D54(D_801F3D4C + cursor[0xB] * 0x11);
  row = (u8)i * 3;
  func_8014D6DC(D_801F3D71[row], D_801F3D72[row]);
  *slot = cursor[4];
  func_801A4D54(D_801F3D4C + cursor[0xB] * 0x11);
  func_8014D6DC(D_801F3D71[row], D_801F3D72[row]);
  D_1F800044 = cursor;

  offset = cursor[3] * 0x98;
  accum = *(s32*)(D_801468BC + offset);
  accum -= 0x1000;
  *(s32*)(D_801468BC + offset) = accum;
  offset = cursor[3] * 0x98;
  accum = *(s32*)(D_801468C0 + offset);
  accum += 0x1000;
  *(s32*)(D_801468C0 + offset) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32*)(D_801468BC + offset);
  accum += 0x1000;
  *(s32*)(D_801468BC + offset) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32*)(D_801468C0 + offset);
  accum -= 0x1000;
  *(s32*)(D_801468C0 + offset) = accum;

  word = D_80145ECC;
  offset = cursor[4] * 0x98;
  *(s32*)(D_801468C4 + offset) = word;
  offset = cursor[3] * 0x98;
  *(s32*)(D_801468C4 + offset) = word;

  offset = cursor[3] * 0x98;
  value = D_80146888[offset];
  value |= 0x20;
  D_80146888[offset] = value;
  offset = D_1F800044[4] * 0x98;
  value = D_80146888[offset];
  value |= 0x20;
  D_80146888[offset] = value;

  pair = D_1F800044;
  offset = pair[4] * 0x98;
  D_801468E4[offset] = 0;
  offset = pair[3] * 0x98;
  D_801468E4[offset] = 0;

  func_8019651C(D_80146888 + cursor[3] * 0x98, 0xF, 0, 0, 1);
  func_8019651C(D_80146888 + cursor[4] * 0x98, 0, 0, 0xF, 1);

  D_1F800044[9] = 4;
  D_1F800044[1] = 1;
}
