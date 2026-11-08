#include "bof3/bof3.h"

void drawRadialGouraudLineBurstAroundPointWorld01Area06714_801F31C8(u8* quad);

extern u8* volatile D_1F800044;

/* @source 0x801F2DA0
 * @behavior State-1 step handler of this overlay work-byte-1 dispatcher (its
 *           pointer is word 1 of the handler table at 0x801F4624): it reads the
 *           scratchpad work record published at pointer cell 0x1F800044 and
 *           steps that record's halfword at +0x10 up by 2 while its countdown
 *           byte at +9 reads 0x1A or above and down by 2 otherwise, then grows
 *           the halfword at +0x12 by 0x40 - the same +6 halfword of the +0xC
 *           quad that the draw helper
 *           drawRadialGouraudLineBurstAroundPointWorld01Area06714_801F31C8
 *           reads - and redraws that quad through that helper with
 *           (record + 0xC). Finally it decrements the
 *           countdown byte at +9 and, once that byte reaches zero, advances the
 *           state byte at +1. The record pointer is re-read from the cell after
 *           the call and again on the advancing path, matching the three
 *           separate lui/lw pairs the original keeps. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkHalfword10By2AtLimit1aGrowHalfword12By40ThenAdvanceWorkByte1World01Area06714_801F2DA0(
    void) {
  u8* work;
  u8* quad;
  u8 count;

  work = D_1F800044;
  quad = work + 0xCu;
  if (work[9] >= 0x1Au) {
    FIELD_REF(u16, work, 0x10u) += 2;
  } else {
    FIELD_REF(u16, work, 0x10u) -= 2;
  }
  FIELD_REF(u16, quad, 0x6u) += 0x40;
  drawRadialGouraudLineBurstAroundPointWorld01Area06714_801F31C8(quad);
  work = D_1F800044;
  count = (u8)(work[9] - 1);
  work[9] = count;
  if (count == 0) {
    D_1F800044[1]++;
  }
}
