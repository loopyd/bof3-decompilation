#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area16813WorkCursor_1F800044;

/* @source 0x801F4158 @kind function */
extern void func_801F4158(s32 index, s8 mode);

/* @source 0x801F3DB0
 * @behavior First handler in the state-byte-2 table at 0x801F4BA0. Sets work
 *           byte +8 to 1, reloads the scratchpad work cursor at 0x1F800044,
 *           clears halfwords +0x3E and +0x30, stores 0x28709126 at word +0x20,
 *           and increments byte +2. Calls the local helper at 0x801F4158 with
 *           (0, 1) and (1, 2). Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWork20ClearWork303eAndAdvanceStepThenCallPairWorld04Area16813_801F3DB0(void) {
  u8* work;

  area16813WorkCursor_1F800044[8] = 1;
  work = area16813WorkCursor_1F800044;
  FIELD_REF(u16, work, 0x3E) = 0;
  FIELD_REF(u32, work, 0x20) = 0x28709126u;
  FIELD_REF(u16, work, 0x30) = 0;
  work[2]++;
  func_801F4158(0, 1);
  func_801F4158(1, 2);
}
