#include "bof3/bof3.h"

extern u8 D_80143FC8[];
s32 func_8019601C(void);

/* @source 0x801F61F0
 * @behavior Asks the shared object selector func_8019601C for an index, masks
 *           the result to a byte and, unless that byte is 0xFF, marks the
 *           selected 0x74-byte front-end record at 0x80143FC8 active (byte +0
 *           = 1) and stores the caller's value in record byte +5. Takes one
 *           argument and returns nothing; the two same-binary wrappers at
 *           0x801F6084 and 0x801F60A4 call it with 74 and 76.
 * @status exact
 * @match 100.00
 * @residual none
 */
void markSelectedFrontEndRecordActiveWithValueWorld02Area07714_801F61F0(
    s32 arg0) {
  s8* record;
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    record = (s8*)(D_80143FC8 + index * 0x74);
    record[0] = 1;
    record[5] = arg0;
  }
}
