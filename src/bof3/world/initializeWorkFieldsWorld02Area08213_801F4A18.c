#include "bof3/bof3.h"

/* @source 0x801F4A18
 * @behavior Initializes the scratch work record loaded from pointer cell
 *           0x1F800044: sets word +0x3C to -0x4C0, word +0x14 to 0x53020,
 *           halfwords +0x32/+0x30 to 0/0x60, and words +0x10/+0x0C to zero.
 *           Saves the old +0x0C/+0x10 words, adds 0x238000/0x1B8000 with
 *           32-bit wrapping, arithmetically shifts each by nine, subtracts
 *           0x4000 and stores the results at +0x34/+0x38. Advances byte +0x02.
 *           Takes no arguments and returns nothing; field roles are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkFieldsWorld02Area08213_801F4A18(void) {
  u8* work;
  s32 x;
  s32 y;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(s32*)(work + 0x3C) = -0x4C0;
  x = (s32)((u32)*(s32*)(work + 0x0C) + 0x238000u);
  y = (s32)((u32)*(s32*)(work + 0x10) + 0x1B8000u);
  *(s32*)(work + 0x14) = 0x53020;
  *(s16*)(work + 0x32) = 0;
  *(s16*)(work + 0x30) = 0x60;
  *(s32*)(work + 0x10) = 0;
  *(s32*)(work + 0x0C) = 0;
  *(s32*)(work + 0x34) = (x >> 9) - 0x4000;
  *(s32*)(work + 0x38) = (y >> 9) - 0x4000;
  work[2]++;
}
