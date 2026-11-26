#include "base/types.h"

extern void renderOffsetTilePair(u32 mode); /* @source 0x801F07B0 @kind function */
extern void renderSpriteRowQuads(u32 mode); /* @source 0x801F0164 @kind function */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE28
 * @behavior Calls overlay helpers 0x801F07B0 and 0x801F0164 in that order,
 * passing 2 to each. Decrements unsigned work byte 9 through the scratchpad
 * pointer cell at 0x1F800044. If the decremented byte is zero, reloads that
 * cell to rearm byte 9 to 10, then reloads it again to advance work byte 1.
 * Its address is installed at 0x801F0AE0 as the third word of the handler
 * pointer run at 0x801F0AD8, so the table carries no prototype and the roles
 * stay structural. The helper roles remain unknown. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callPairDecrementRearm(void) {
  u8* work;
  u8* rearm;
  u8* step;

  renderOffsetTilePair(2);
  renderSpriteRowQuads(2);
  work = g_battle_work;
  if (--work[9] == 0) {
    rearm = g_battle_work;
    rearm[9] = 10;
    step = g_battle_work;
    step[1]++;
  }
}
