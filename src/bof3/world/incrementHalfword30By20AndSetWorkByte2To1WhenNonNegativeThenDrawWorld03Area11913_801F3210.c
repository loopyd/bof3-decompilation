#include "bof3/bof3.h"

extern u8* world03Area11913WorkRecord; /* @source 0x1F800044 @kind data */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */
extern void func_801F301C(void); /* @source 0x801F301C @kind function */
extern void func_801F3324(void); /* @source 0x801F3324 @kind function */
extern void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 @kind function */

/* @source 0x801F3210
 * @behavior Adds 0x20 to the work record halfword at offset 0x30 and stores
 *           the truncated value. When that value is nonnegative as a signed
 *           halfword, calls the overlay camera-window handler at 0x801F301C,
 *           reloads the scratchpad work pointer and returns if state byte 2
 *           is 2. Otherwise sets that byte to 1 and queues cue 0x201 through
 *           0x8015DF18 when the shared phase byte at 0x80143BB0 is zero.
 *           All paths except that early return call the record draw routine
 *           at 0x801F3324. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementHalfword30By20AndSetWorkByte2To1WhenNonNegativeThenDrawWorld03Area11913_801F3210(
    void) {
  u8* work;
  u8* resumedWork;
  u16 half;

  work = world03Area11913WorkRecord;
  half = *(u16*)(work + 0x30) + 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half >= 0) {
    func_801F301C();
    resumedWork = world03Area11913WorkRecord;
    if (resumedWork[2] == 2) {
      return;
    }
    resumedWork[2] = 1;
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
  }
  func_801F3324();
}
