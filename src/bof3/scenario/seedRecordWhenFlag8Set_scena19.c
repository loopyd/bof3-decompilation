#include "bof3/bof3.h"

extern u16 D_80145AA8;
extern u8 D_80143FC8[];
s32 func_8019601C(void);

/* @source 0x801F6CA4
 * @behavior If D_80145AA8 has mask 0x8 set, calls func_8019601C.
 * Masks its result to a byte and, unless that byte is 0xFF, marks the selected
 * record at D_80143FC8 with stride 0x74, storing 1 at +0 and 0x2F at +5.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRecordWhenFlag8Set_scena19(void) {
  s8* record;
  s32 index;

  if (D_80145AA8 & 8) {
    index = func_8019601C() & 0xFF;
    if (index != 0xFF) {
      record = (s8*)(D_80143FC8 + index * 0x74);
      record[0] = 1;
      record[5] = 0x2F;
    }
  }
}
