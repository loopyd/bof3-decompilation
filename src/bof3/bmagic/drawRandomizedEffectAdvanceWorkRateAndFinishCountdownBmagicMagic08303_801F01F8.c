#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801F0BA8(s32 scale); /* @source 0x801F0BA8 @kind function */
extern void func_801EF794(void); /* @source 0x801EF794 @kind function */
extern void func_801F050C(void); /* @source 0x801F050C @kind function */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C @kind function */

/* @source 0x801F01F8
 * @behavior Handler referenced by the table word at 0x801F0F14. Calls the
 *           shared helper at 0x800AF764, draws through 0x801F0BA8 with random
 *           scale 64 to 67 when work word 0xC is 4 and byte 0xA is below 5,
 *           otherwise 24 to 27, installs the work transform through 0x801EF794,
 *           invokes 0x801F050C and pops the matrix. While signed word 0xC is
 *           below 4, increments word 0x10 and increments word 0xC every eighth
 *           tick. Adds word 0xC into byte 9. At word 0xC equal to 4, decrements
 *           byte 0xA; at zero, calls 0x8015D71C with 0x101 or 0x102 if byte 3
 *           has no high-nibble bits, resets words 0x10, 0x14 and 0x20 to 8,
 *           and advances byte 2. Takes no arguments and returns nothing.
 * @status partial
 * @match 87.13
 * @residual Middle rate-update block has a different pointer/value register web.
 */
void drawRandomizedEffectAdvanceWorkRateAndFinishCountdownBmagicMagic08303_801F01F8(void) {
  u8 *work;
  u8 flags;
  u8 *reset;

  func_800AF764();
  work = D_1F800044;
  if (*(s32 *)(work + 0xC) == 4 && work[0xA] < 5) {
    func_801F0BA8((rand() & 3) | 64);
  } else {
    func_801F0BA8((rand() & 3) | 24);
  }
  func_801EF794();
  func_801F050C();
  PopMatrix();
  work = D_1F800044;
  if (*(s32 *)(work + 0xC) < 4) {
    *(s32 *)(work + 0x10) += 1;
    if ((*(s32 *)(work + 0x10) & 7) == 0) {
      *(s32 *)(work + 0xC) += 1;
    }
    work = D_1F800044;
  }
  work[9] += *(s32 *)(work + 0xC);
  work = D_1F800044;
  if (*(s32 *)(work + 0xC) == 4) {
    work[0xA]--;
    if (work[0xA] == 0) {
      flags = D_1F800044[3];
      if ((flags & 0xF0) == 0) {
        func_8015D71C((flags & 1) ? 0x101 : 0x102);
      }
      reset = D_1F800044;
      *(s32 *)(reset + 0x10) = 8;
      *(s32 *)(reset + 0x14) = 8;
      *(s32 *)(reset + 0x20) = 8;
      reset[2]++;
    }
  }
}
