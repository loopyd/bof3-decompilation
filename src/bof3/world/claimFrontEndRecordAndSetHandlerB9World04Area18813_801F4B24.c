#include "bof3/bof3.h"

/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8. The original materialises the base once, adds the scaled row
 * offset and then reuses that address for both stores, so the source keeps one
 * row pointer and the two byte stores through it.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

/* Record allocator of the shared EXE image, outside this payload: it answers
 * the index of a free 0x74-byte row of the front-end record table, or 0xFF when
 * every row is occupied. The original masks the answer to a byte before the
 * 0xFF test and before the row chain, so this overlay sees the return as a
 * whole word.
 */
s32 func_8019601C(void);

/* @source 0x801F4B24
 * @behavior Overlay record seed: asks the shared free-record selector
 *           func_8019601C for an index, masks the answer to a byte and, unless
 *           that byte reads 0xFF, marks the selected 0x74-byte front-end record
 *           of the shared table at 0x80143FC8 occupied (record byte +0 = 1) and
 *           stores handler selector 0xB9 in record byte +5. It takes no
 *           arguments and returns nothing: the 0x18-byte frame saves $ra at
 *           0x10 and the row chain folds to index * 0x74 from the masked byte.
 *           No jal and no table word of this payload names the address, so the
 *           overlay loader is the only caller and the roles stay structural.
 *           The 0x5C-byte span measured byte-identical to the span at the same
 *           offset of the world03 area121 slot 14 overlay except for that one
 *           stored literal, which reads 0xBA there.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordAndSetHandlerB9World04Area18813_801F4B24(void) {
  u8* record;
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    record = (u8*)(D_80143FC8 + index * 0x74);
    record[0] = 1;
    record[5] = 0xB9;
  }
}
