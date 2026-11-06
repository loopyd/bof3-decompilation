#include "bof3/bof3.h"

extern u8 *D_1F800044;
/* @source 0x80143FC8 @kind table */
extern u8 D_80143FC8[];

/* @source 0x801F7E6C
 * @behavior Entry 1 of the overlay's three-word work-object handler table
 * workByte1HandlersScenarioScena0300_801FCF88 (0x801F7E34 / 0x801F7E6C / this
 * function), which the frame dispatcher func_801F7DF0 selects with byte 0x1 of
 * the work object published at scratchpad pointer slot 0x1F800044 and calls
 * with no arguments: it claims five front-end records into that work object's
 * five record-pointer slots at offset 0x0C, taking one record index per
 * iteration from func_8019601C and keeping the slot untouched when that index
 * reads 0xFF. The claimed record is row (index * 0x74) of the front-end record
 * table at 0x80143FC8: byte 0x00 is set to 1, byte 0x06 to 0, byte 0x05 to
 * 0x21, the words at 0x0C, 0x10 and 0x14 are copied from the work object's
 * words 0x34, 0x38 and 0x3C, halfword 0x58 is set to 6, halfwords 0x5A and
 * 0x5E to 0, and halfword 0x5C to the truncated product (index * 0x333). The
 * pointer cell is re-read for each iteration's word copies, because the index
 * helper may republish it, and once more after the loop, where the function
 * advances work byte 0x01 by one so the next frame dispatches entry 2
 * (0x801F7F84), which waits until every claimed record reads a non-zero byte
 * 0x06. Takes no arguments, returns nothing and reads no other state; the
 * 0x20-byte frame keeps $ra, $s0 (the byte-masked loop counter, also the slot
 * index) and $s1 (the work object's slot base) across the index call. The
 * roles of record bytes 0x05/0x06 and of halfwords 0x58-0x5E beyond entry 2's
 * test are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape note from the original bytes: the pointer cell is read inline at each
 * of the three word copies and again for the trailing byte increment. Copying
 * the cell into a pointer local once per iteration measured 52/70 with the
 * same size and instruction order, because the local's reload was tied to the
 * result of the word-0x3C load and the byte-masked slot index then took the
 * neighbouring register (measured 52/70); the inline spelling restores the
 * original split of the reload, the third word and the masked index.
 */
void claimFiveFrontEndRecordsAndAdvanceWorkByte1ScenarioScena0300_801F7E6C(
    void) {
  u8 *work;
  u8 **records;
  u8 *record;
  u32 word;
  u8 index;
  u8 i;

  work = D_1F800044;
  records = (u8 **)(work + 0x0C);
  for (i = 0; i < 5; i++) {
    index = func_8019601C();
    if (index != 0xFFu) {
      record = &D_80143FC8[index * 0x74];
      records[i] = record;
      record[0] = 1;
      record[6] = 0;
      record[5] = 0x21;
      *(u32 *)(record + 0x0C) = *(u32 *)(D_1F800044 + 0x34);
      *(u32 *)(record + 0x10) = *(u32 *)(D_1F800044 + 0x38);
      word = *(u32 *)(D_1F800044 + 0x3C);
      *(u16 *)(record + 0x58) = 6;
      *(u16 *)(record + 0x5A) = 0;
      *(u16 *)(record + 0x5C) = (u16)(i * 0x333);
      *(u16 *)(record + 0x5E) = 0;
      *(u32 *)(record + 0x14) = word;
    }
  }
  D_1F800044[1]++;
}
