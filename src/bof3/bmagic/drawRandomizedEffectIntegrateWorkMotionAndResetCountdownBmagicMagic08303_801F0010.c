#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801F0BA8(s32 scale); /* @source 0x801F0BA8 @kind function */
extern void func_801EF794(void); /* @source 0x801EF794 @kind function */
extern void func_801F050C(void); /* @source 0x801F050C @kind function */

/* @source 0x801F0010
 * @behavior Handler referenced by the table word at 0x801F0F0C. Calls the
 *           shared helper at 0x800AF764, draws through 0x801F0BA8 with a random
 *           scale from 24 to 27, installs the work transform through 0x801EF794,
 *           invokes 0x801F050C and pops the matrix. On odd frames increments
 *           work byte 9. Decrements byte 0xA, adds word 0x20 into word 0x14,
 *           and accumulates the result into halfword 0x3E. At countdown zero,
 *           resets word 0x14 to -16, byte 0xA to 8 and word 0x20 to 4, then
 *           increments work byte 2 through a reloaded scratchpad pointer.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRandomizedEffectIntegrateWorkMotionAndResetCountdownBmagicMagic08303_801F0010(void) {
  u8 *work;
  u8 count;
  u8 *reset;
  u8 *step;

  func_800AF764();
  func_801F0BA8((rand() & 3) | 24);
  func_801EF794();
  func_801F050C();
  PopMatrix();
  if (D_80143E6C & 1) {
    D_1F800044[9]++;
  }
  work = D_1F800044;
  count = (u8)(work[0xA] - 1);
  *(s32 *)(work + 0x14) += *(s32 *)(work + 0x20);
  *(u16 *)(work + 0x3E) += *(s32 *)(work + 0x14);
  work[0xA] = count;
  if (count == 0) {
    reset = D_1F800044;
    *(s32 *)(reset + 0x14) = -16;
    reset[0xA] = 8;
    step = D_1F800044;
    *(s32 *)(reset + 0x20) = 4;
    step[2]++;
  }
}
