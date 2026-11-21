#include "bof3/bof3.h"

extern u8 D_80143FC8[];
s32 func_8019601C(void);

/* @source 0x801F2E1C
 * @behavior Asks the shared free-record selector func_8019601C for an index,
 * masks the result to a byte and, unless that byte is 0xFF, marks the selected
 * 0x74-byte front-end record at 0x80143FC8 occupied (byte +0 = 1) and writes
 * handler selector 0x44 into record byte +5. Takes no arguments and returns
 * nothing: it opens on its own frame and closes on the shared epilogue, and the
 * record offset is formed as index*8 - index, scaled by 4, plus index, scaled
 * by 4 again, which is the 0x74 stride. The only in-image reference is the
 * ninth word of the overlay dispatch table based at 0x801F411C, whose earlier
 * code entries hold 0x801F2C04, 0x801F2C68, 0x801F2C70, 0x801F2C78,
 * 0x801F2CB8, 0x801F2CF8, 0x801F2D38, 0x801F2D78 and this address.
 * Bytes 0x21C..0x278 of this payload are identical to the 92 bytes at
 * 0x801F3D50 in emi/world02/area080/13, where
 * claimFrontEndRecordAndSetHandler44World02Area08013_801F3D50 recorded a
 * matched lift, so this body transcribes that sibling; its call target and
 * record-table base both resolve in the shared main executable, so no word of
 * the two spans differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordAndSetHandler44World02Area07813_801F2E1C(void) {
  u8* record;
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    record = (u8*)(D_80143FC8 + index * 0x74);
    record[0] = 1;
    record[5] = 0x44;
  }
}
