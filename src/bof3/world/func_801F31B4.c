#include "bof3/world/area02414_internal.h"

/**
 * @source 0x801F31B4
 * @behavior Advances the current work entry by twice its three velocity
 * components, decrements its byte timer, advances its halfword phase, and
 * moves to the next state with a reset timer when the timer expires.
 */
void func_801F31B4(void) {
  World00Area024SpriteWork* work;
  u8 timer;

  work = (World00Area024SpriteWork*)workCursor;
  timer = work->field_02 - 1;
  work->field_04 += work->field_14.vx * 2;
  work->field_08 += work->field_14.vy * 2;
  work->field_0c += work->field_14.vz * 2;
  work->field_02 = timer;
  work->field_24 += 0x80;

  if (timer == 0) {
    ((World00Area024SpriteWork*)workCursor)->field_02 = 0x40;
    ((World00Area024SpriteWork*)workCursor)->field_01++;
  }
}
