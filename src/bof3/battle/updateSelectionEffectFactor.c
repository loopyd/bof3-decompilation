#include "bof3/battle/battle03_internal.h"

extern u8 D_80145BD4[]; /* @source 0x80145BD4 @kind unknown */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind unknown */
void func_80196670(u8 arg0); /* @source 0x80196670 */

/* @source 0x801DF7EC
 * @behavior Copies the oscillating factor byte into bytes 4, 3 and 2 of the
 * 12-byte effect record selected by scratch work byte 7. If the selection gate
 * is clear, or the active selection neither matches scratch work byte 5 nor
 * carries bit 0x80, submits the effect byte to func_80196670 and resets scratch
 * state byte 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateSelectionEffectFactor(void) {
  u32 offset;
  u8 factor;
  u8* record;

  offset = battleWork[7] * 12;
  factor = BATTLE_GLOBAL_BYTE_6308;
  record = &D_80145BD4[offset];
  record[4] = factor;
  record[3] = factor;
  D_80145BD6[offset] = factor;
  if (D_801462EF != 0) {
    if (*D_801EB4D8 == battleWork[5] || (*D_801EB4D8 & 0x80) != 0) {
      return;
    }
  }
  func_80196670(battleWork[7]);
  battleWork[2] = 0;
}
