#include "bof3/bof3.h"

extern u8 * volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF55C
 * @behavior Adds work words 0xC and 0x10 into words 0x34 and 0x38,
 * increments byte 9 by one, then reloads the scratchpad work pointer to
 * increment byte 0xA by two. Reloads the pointer again and increments byte 2
 * when byte 0xA equals 0x10. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkMotionPairAndIncrementBytes9AThenAdvanceWorkByte2At10BmagicMagic04303_801EF55C(void) {
  u32 *words;
  u8 *step;
  u8 *state;

  words = (u32 *)g_battle_work;
  words[13] += words[3];
  ((u8 *)words)[9]++;
  words[14] += words[4];
  step = g_battle_work;
  step[0xA] += 2;
  state = g_battle_work;
  if (state[0xA] == 0x10) {
    state[2]++;
  }
}
