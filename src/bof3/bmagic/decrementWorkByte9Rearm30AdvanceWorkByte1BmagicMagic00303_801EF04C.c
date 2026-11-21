#include "base/types.h"

extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF04C
 * @behavior Decrements unsigned work byte 9 through the scratchpad pointer
 * cell at 0x1F800044. If the decremented byte is zero, reloads that cell to
 * rearm byte 9 to 30, then reloads it again to advance work byte 1. The body
 * is entry 5 of the handler pointer run at 0x801F0AD4, so the table carries
 * no prototype and the roles stay structural. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9Rearm30AdvanceWorkByte1BmagicMagic00303_801EF04C(void) {
  u8* work;
  u8* rearm;
  u8* step;

  work = g_battle_work;
  if (--work[9] == 0) {
    rearm = g_battle_work;
    rearm[9] = 30;
    step = g_battle_work;
    step[1]++;
  }
}
