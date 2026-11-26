#include "base/types.h"

extern void renderOffsetTilePair(u32 mode); /* @source 0x801F07B0 @kind function */
extern void renderSpriteRowQuads(u32 mode); /* @source 0x801F0164 @kind function */
extern void renderGreyDecimalLeft(u8 value);
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0A8
 * @behavior Calls overlay helper 0x801F07B0 with 0, then reads work byte 0xA
 * through the scratchpad pointer cell at 0x1F800044 and passes it to the grey
 * decimal-pair drawer. Calls overlay helper 0x801F0164 with 0, then decrements
 * unsigned work byte 9 and, when the decremented byte is zero, reloads that
 * cell to advance work byte 1. The body is entry 7 of the handler pointer run
 * at 0x801F0AD4, so the table carries no prototype and the roles stay
 * structural. Takes no arguments and returns nothing. Both helper roles remain
 * unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderDecimalPairDecrement(void) {
  u8* work;
  u8* step;

  renderOffsetTilePair(0);
  renderGreyDecimalLeft(g_battle_work[0xA]);
  renderSpriteRowQuads(0);
  work = g_battle_work;
  if (--work[9] == 0) {
    step = g_battle_work;
    step[1]++;
  }
}
