#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801469EA @kind unknown */
extern u16 workSeedHalfword801469EA;
/* @source 0x801F3400 @kind function */
extern void func_801F3400(s32 value2e, s32 value30, s32 value0c, s32 value10);

/* @source 0x801F3284
 * @behavior Overlay step handler: entry 4 of the seven-word handler run based
 * at 0x801F3ED8 that this target's exact dispatcher
 * callStateTable3World03 indexes with the work
 * record's state byte at +1. Adds 0xF00 to the shared seed halfword at
 * 0x801469EA, then steps the published work record: adds 0xF to its word at
 * +0x0C and 0xA to its word at +0x10, and invokes 0x801F3400 with the record's
 * signed halfwords at +0x2E, +0x30, +0x0C and the truncated low half of the
 * stepped word at +0x10. Reloads the published record after the call,
 * decrements its countdown byte at +0x09, and when that countdown reaches zero
 * reloads the record twice more to rearm the countdown with 0x3C and to advance
 * the state byte at +0x01, so the next frame dispatches entry 5. Takes no
 * arguments and returns nothing. The roles of the seed halfword, of the stepped
 * words and of the rearmed countdown stay described by offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateHalfwordTickReset(
    void) {
  u16* seed;
  u8* work;
  u8* countdown;
  u8* state;
  s32 word0c;
  s32 word10;

  seed = &workSeedHalfword801469EA;
  *seed += 0xF00;
  work = workRecordCursor1F800044;
  word0c = *(s32*)(work + 0x0C) + 0xF;
  word10 = *(s32*)(work + 0x10) + 0xA;
  *(s32*)(work + 0x0C) = word0c;
  *(s32*)(work + 0x10) = word10;
  func_801F3400(*(s16*)(work + 0x2E), *(s16*)(work + 0x30),
                *(s16*)(work + 0x0C), (s16)word10);
  countdown = workRecordCursor1F800044;
  if (--countdown[9] == 0) {
    countdown = workRecordCursor1F800044;
    countdown[9] = 0x3C;
    state = workRecordCursor1F800044;
    state[1] += 1;
  }
}
