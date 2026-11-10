#include "base/types.h"

extern void func_801F07B0(u32 mode); /* @source 0x801F07B0 @kind function */
extern void func_801F0164(u32 mode); /* @source 0x801F0164 @kind function */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEDAC
 * @behavior Calls overlay helpers 0x801F07B0 and 0x801F0164 in that order,
 * passing 1 to each. Decrements unsigned work byte 9 through the scratchpad
 * pointer cell at 0x1F800044. If the decremented byte is zero, reloads that
 * cell to rearm byte 9 to 30, then reloads it again to advance work byte 1.
 * Takes no arguments and returns nothing. The helper roles remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runHelpers1DecrementWorkByte9Rearm30AdvanceWorkByte1BmagicMagic00303_801EEDAC(void) {
  u8* work;
  u8* rearm;
  u8* step;

  func_801F07B0(1);
  func_801F0164(1);
  work = g_battle_work;
  if (--work[9] == 0) {
    rearm = g_battle_work;
    rearm[9] = 30;
    step = g_battle_work;
    step[1]++;
  }
}
