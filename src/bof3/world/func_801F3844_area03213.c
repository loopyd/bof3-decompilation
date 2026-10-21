#include "bof3/world/area03213_internal.h"

/* @source 0x801F3844
 * @behavior Boots the scratch cursor record at 0x1F800044: submits the
 * positional effect through func_8019651C and stores the submitted effect byte
 * at 0x93 of the current work record at 0x80146884, sets record flag bit 0x20,
 * clears flag bit 0x40, sets the 0x5C byte and the -0x80 level triple at
 * 0x5D-0x5F, resets the 0x0C step to 0x2000, advances the 0x34 accumulator by
 * 0x2000, selects handler byte 1, and consumes two units of the countdown word
 * at 0x7E of the work record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3844(void) {
  u8* record;
  u8* cursor;
  u8* work;

  D_80146884[0x93] = func_8019651C(D_1F800044, 0, 0, 0, 1);
  D_1F800044[0] |= 0x20;
  D_1F800044[0] &= 0xbf;
  D_1F800044[0x5c] = 1;
  record = D_1F800044;
  *(s8*)(record + 0x5f) = -0x80;
  *(s8*)(record + 0x5e) = -0x80;
  *(s8*)(record + 0x5d) = -0x80;
  cursor = D_1F800044;
  *(s32*)(cursor + 0x0c) = 0x2000;
  cursor[4] = 1;
  work = D_80146884;
  *(s32*)(cursor + 0x34) += 0x2000;
  *(u16*)(work + 0x7e) -= 2;
}
