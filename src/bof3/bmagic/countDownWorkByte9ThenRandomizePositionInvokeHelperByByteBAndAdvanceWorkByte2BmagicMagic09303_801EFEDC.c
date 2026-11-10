#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EFEDC
 * @behavior Decrement work byte 9 and return until it reaches zero. Copy words
 *           0x34, 0x38 and 0x3C from the record at 0x801EB4E0, then offset the
 *           first two words by angular samples times radius 0x100 shifted
 *           right three. Publish the radius and random angle (rand() & 7)
 *           shifted left nine to scratchpad halfwords 0 and A. If work byte B
 *           is odd, invoke helper 0x8015D71C with 0x101, 0x102 or 0x103 selected
 *           by (byte B >> 1) modulo three. Set bytes B, 9 and A to 0x10, 0 and
 *           0x40, then advance byte 2. Re-read the work pointer between access
 *           groups. The entry is table word 0x801F08C4 with no direct in-payload
 *           call; it takes no arguments and returns nothing. Helper and field
 *           roles remain address- or offset-anchored.
 * @status partial
 * @match 93.81
 * @residual Seven register substitutions in the second angular-result update;
 *           106 of 113 instructions match at the original 452-byte size.
 */
void countDownWorkByte9ThenRandomizePositionInvokeHelperByByteBAndAdvanceWorkByte2BmagicMagic09303_801EFEDC(void) {
  u8 *record;
  u8 *work;
  s16 *radius;
  s16 angle;
  s32 sample;
  u8 selector;
  s32 request;

  if (--g_battle_work[9] == 0) {
    record = D_801EB4E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
    *(s32 *)(work + 0x3C) = *(s32 *)(record + 0x3C);
    radius = &D_1F800000;
    *radius = 0x100;
    angle = rand() & 7;
    D_1F80000A = angle << 9;
    sample = func_801782FC(angle << 9);
    *(s32 *)(g_battle_work + 0x34) += (sample * *radius) >> 3;
    sample = func_801783C8(D_1F80000A);
    work = g_battle_work;
    *(s32 *)(work + 0x38) += (sample * *radius) >> 3;
    if (work[0xB] & 1) {
      selector = (work[0xB] >> 1) % 3;
      switch (selector) {
        case 0:
          request = 0x101;
          break;
        case 1:
          request = 0x102;
          break;
        case 2:
        default:
          request = 0x103;
          break;
      }
      func_8015D71C(request);
    }
    g_battle_work[0xB] = 0x10;
    g_battle_work[9] = 0;
    g_battle_work[0xA] = 0x40;
    g_battle_work[2]++;
  }
}
