#include "bof3/bof3.h"

extern u8 D_80143FC8[];
s32 func_8019601C(void);

/* @source 0x801F3D50
 * @behavior Asks the shared free-record selector func_8019601C for an index,
 * masks the result to a byte and, unless that byte is 0xFF, marks the selected
 * 0x74-byte front-end record at 0x80143FC8 occupied (byte +0 = 1) and writes
 * handler selector 0x44 into record byte +5. Takes no arguments and returns
 * nothing; the only in-image reference is the eighth word of the code-pointer
 * run based at 0x801F4430, which holds 0x801F4400, 0x801F3BF4, 0x801F3C44,
 * 0x801F3C94, 0x801F3CE4, 0x801F3D0C, 0x801F3D34 and this address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordAndSetHandler44World02Area08013_801F3D50(void) {
  u8* record;
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    record = (u8*)(D_80143FC8 + index * 0x74);
    record[0] = 1;
    record[5] = 0x44;
  }
}
