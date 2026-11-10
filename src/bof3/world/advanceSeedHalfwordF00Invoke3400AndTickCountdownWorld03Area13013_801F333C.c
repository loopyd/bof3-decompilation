#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801469EA @kind unknown */
extern u16 workSeedHalfword801469EA;
/* @source 0x801F3400 @kind function */
extern void func_801F3400(s32 value2e, s32 value30, s32 value0c, s32 value10);

/* @source 0x801F333C
 * @behavior Adds 0xF00 to the shared seed halfword at 0x801469EA, then
 * invokes 0x801F3400 with the work record's signed halfwords at +0x2E,
 * +0x30, +0x0C and +0x10. Reloads the published work record after the call,
 * decrements its countdown byte at +9, and advances the state byte at +1
 * through a fresh record-pointer load when the countdown reaches zero.
 * Takes no arguments and returns nothing. The seed's role remains unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceSeedHalfwordF00Invoke3400AndTickCountdownWorld03Area13013_801F333C(void) {
  u16* seed;
  u8* work;
  u8* countdown;
  u8* state;

  seed = &workSeedHalfword801469EA;
  *seed += 0xF00;
  work = workRecordCursor1F800044;
  func_801F3400(*(s16*)(work + 0x2E), *(s16*)(work + 0x30),
                *(s16*)(work + 0x0C), *(s16*)(work + 0x10));
  countdown = workRecordCursor1F800044;
  if (--countdown[9] == 0) {
    state = workRecordCursor1F800044;
    state[1] += 1;
  }
}
