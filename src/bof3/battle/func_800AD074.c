#include "bof3/battle/battle15_internal.h"

/* @behavior sets bit (arg0 & 0x1F) of the bitmask word D_80144F60[arg0 >> 5].
 * @source 0x800AD074
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800AD074(u16 arg0) {
  s32 bit_mask;

  /*
   * MATCHING_AID:
   * Permuter-found: the named 0x1F constant stops GCC's combine pass from
   * folding (arg0 >> 5) << 2 into (arg0 >> 3) & 0x1FFC; the original keeps
   * andi $a0,0xffff / srl $a1,$a0,5 / sll $a1,$a1,2 unfolded. Remove when the
   * original compiler's combine behavior for this idiom is understood.
   */
  bit_mask = 0x1F;
  D_80144F60[arg0 >> 5] |= (1U << (arg0 & bit_mask));
}
