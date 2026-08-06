#include "internal.h"

/* @behavior builds a 0/0x7FFF flag from the sign of the first argument, then
 * retries drawFadeTile until it reports success, waiting one countdown tick
 * per failure. On a positive count it clears two three-byte state arrays,
 * then unconditionally exits the current callback thread.
 * @source 0x8014F514
 */
void fadeLoop(s32 a0, s32 a1, s32 a2) {
  u16 p0 = a0;
  s32 p1 = a1;
  s32 p2 = a2;
  s32 t;
  s16 value;
  s32 n;

  t = a0 << 16;
  if (t <= 0) {
    value = 0x7FFF;
  } else {
    value = 0;
  }
  n = (s16)p0;
  for (;;) {
    if (drawFadeTile(&value, n, p1, p2, 2) != 0) {
      break;
    }
    yieldCallbackSlotScheduler(1);
  }
  if (n > 0) {
    D_80143E05[2] = 0;
    D_80143E05[1] = 0;
    D_80143E05[0] = 0;
    D_80143D75[2] = 0;
    D_80143D75[1] = 0;
    D_80143D75[0] = 0;
  }
  exitCurrentCallbackThread();
}
