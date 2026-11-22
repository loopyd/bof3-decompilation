#include "base/types.h"

extern void func_801F07B0(u32 mode); /* @source 0x801F07B0 @kind function */
extern void func_801F0164(u32 mode); /* @source 0x801F0164 @kind function */
extern void drawGreyDecimalPairLeftOfWorkScreenPairBmagicMagic00303_801F0550(u8 value);
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
void drawWorkByteADecimalPairThenDecrementWorkByte9AdvanceWorkByte1BmagicMagic00303_801EF0A8(void) {
  u8* work;
  u8* step;

  func_801F07B0(0);
  drawGreyDecimalPairLeftOfWorkScreenPairBmagicMagic00303_801F0550(g_battle_work[0xA]);
  func_801F0164(0);
  work = g_battle_work;
  if (--work[9] == 0) {
    step = g_battle_work;
    step[1]++;
  }
}
