#include "bof3/ui/game00_internal.h"

/*
 * @behavior when world flag byte D_80143F02 bit 0 is set, walks the first
 *           D_80146254 records of the 0x140-byte-stride table whose leading byte
 *           lives at D_80145FCC and tests bit 5 of the flags word of the 164-byte
 *           game input record in D_80144974 selected by that leading byte; with
 *           bit 0 clear only the first record is tested. When any tested record
 *           carries the flag, publishes D_80145AA4 into D_8014625C with bits
 *           12-13 and 14-15 swapped, otherwise copies D_80145AA4 unchanged.
 * @source 0x801BDBC4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801BDBC4(void) {
  u8 found = 0;

  if ((D_80143F02 & 1) != 0) {
    s32 index;

    for (index = 0; index < D_80146254; index++) {
      s32 offset = index * 0x140;

      if ((D_80144974[D_80145FCC[offset]].flags & 0x20) != 0) {
        found = 1;
      }
    }
  } else if ((D_80144974[D_80145FCC[0]].flags & 0x20) != 0) {
    found = 1;
  }

  if (found != 0) {
    u16 value = D_80145AA4;

    D_8014625C = (u16)((value & 0x9FF) |
                       (((value & 0x3000) << 2) | ((value & 0xC000) >> 2)));
  } else {
    D_8014625C = D_80145AA4;
  }
}
