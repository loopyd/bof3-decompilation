#include "bof3/bof3.h"

/* Pool of 64 records of stride 0x78 that this overlay steps as battle work
 * objects. The flags byte of each record opens the record, so the base binds at
 * its own word inside the textbin run that starts at 0x801EFC2C. */
extern u8 D_801EFC60[]; /* @source 0x801EFC60 @kind data */

/* @source 0x801EFB78
 * @behavior Reserves the first of 64 records in the pool at 0x801EFC60 whose
 *           flags byte has bit 0 clear, sets that bit and returns the record
 *           index, or returns 0xFF when every record is occupied. Records have
 *           stride 0x78. The only caller, the jal at 0x801EEE34 in
 *           func_801EED18, masks the result to a byte, compares it with 0xFF and
 *           indexes the same pool with the same stride, writing record byte 0x1,
 *           record byte 0xB and record word 0x74. Takes no arguments, keeps no
 *           frame and touches only the selected record's flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic22303_801EFB78(void) {
  u8 index;
  u8 flags;
  u32 offset;

  for (index = 0; index < 0x40; index++) {
    offset = index * 0x78;
    flags = D_801EFC60[offset];
    if (!(flags & 1)) {
      D_801EFC60[offset] = flags | 1;
      return index;
    }
  }
  return 0xFF;
}
