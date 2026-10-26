#include "bof3/battle/battle15_internal.h"

/* @behavior sets bit (arg0 & 0x1F) of the bitmask word D_80144F60[arg0 >> 5].
 * @source 0x800AD074
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800AD074(u16 bitIndex) {
  s32 bit_mask;

  bit_mask = 0x1F;
  D_80144F60[bitIndex >> 5] |= (1U << (bitIndex & bit_mask));
}
