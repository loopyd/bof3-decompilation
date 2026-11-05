#include "bof3/bof3.h"

extern u16 D_80146258;
extern u16 D_8014625C;
extern u16 D_80145ABA;
extern u8 D_80143BB0;

/* @source 0x801F55EC
 * @behavior Returns zero when halfword 0x80146258 has bit 0x2000 set or
 * halfwords 0x8014625C and 0x80145ABA have no common set bits. Otherwise
 * writes 4 to byte 0x80143BB0, clears byte 2 of the work record in
 * scratchpad pointer slot 0x44, and returns one. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setMode4AndClearWorkByte2IfMaskOverlapWorld03Area12114_801F55EC(void) {
  u8* record;

  if ((D_80146258 & 0x2000u) == 0) {
    if ((D_8014625C & D_80145ABA) != 0) {
      record = SPAD_PTR_SLOT(u8, 0x44);
      D_80143BB0 = 4;
      record[2] = 0;
      return 1;
    }
  }
  return 0;
}
