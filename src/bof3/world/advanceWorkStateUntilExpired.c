#include "bof3/world/area02414_internal.h"

/**
 * @source 0x801F3250
 * @behavior Advances the current work entry by its three velocity fields,
 * decrements two byte timers, and clears the active flag when timer +2 expires.
 * @status partial
 * @match 48.48
 * @residual non-exact live audit: 16/33 instructions; 132 original bytes versus 132 current.
 */
void advanceWorkStateUntilExpired(void) {
  u8* initialWork;
  u8* timerWork;
  u8* clearWork;
  u8  timer3;

  initialWork = workCursor;
  timer3 = initialWork[3];
  *(s32*)(initialWork + 4) =
      *(s32*)(initialWork + 4) + *(s32*)(initialWork + 20);
  *(s32*)(initialWork + 8) =
      *(s32*)(initialWork + 8) + *(s32*)(initialWork + 24);
  initialWork[3] = timer3 - 1;
  timerWork = workCursor;
  *(s32*)(initialWork + 12) =
      *(s32*)(initialWork + 12) + *(s32*)(initialWork + 28);
  timerWork[2]--;
  if (timerWork[2] == 0) {
    clearWork = workCursor;
    clearWork[0] = 0;
  }
}
