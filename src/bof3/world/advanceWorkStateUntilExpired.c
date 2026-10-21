#include "bof3/world/area02414_internal.h"

/**
 * @source 0x801F3250
 * @behavior Advances the current work entry's three 32-bit position words at
 * +0x04/+0x08/+0x0c by the matching velocity component of its +0x14 vector and
 * decrements its byte-3 timer; it then reloads the work cursor, decrements the
 * entry's byte-2 timer and, when that timer reaches zero, reloads the cursor
 * once more and clears the entry's active byte 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkStateUntilExpired(void) {
  World00Area024SpriteWork* work;
  u8 timer;

  work = (World00Area024SpriteWork*)workCursor;
  work->field_04 += work->field_14.vx;
  work->field_08 += work->field_14.vy;
  work->field_0c += work->field_14.vz;
  work->field_03--;

  timer = ((World00Area024SpriteWork*)workCursor)->field_02 - 1;
  ((World00Area024SpriteWork*)workCursor)->field_02 = timer;
  if (timer == 0) {
    ((World00Area024SpriteWork*)workCursor)->field_00 = 0;
  }
}
