#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s32 func_801782FC(s32 angle);
extern s32 func_801783C8(s32 angle);

/* @source 0x801EF4C8
 * @behavior Seed work byte A with (rand() & 3) << 4, then publish radius
 *           0xC0 minus byte A and angle (byte 4 & 7) << 9 to scratchpad
 *           halfwords 0 and A. Offset work words 0x34 and 0x38 from the
 *           corresponding words of the record at 0x801EB4E0 by each angular
 *           sample times the signed radius, shifted right three. Copy record
 *           word 0x3C, seed byte B from rand(), clear byte 9 and advance byte 2.
 *           Re-read the work pointer between access groups. The entry occurs
 *           at table word 0x801F08B4 with no direct in-payload call; it takes
 *           no arguments and returns nothing. Field roles remain offset-based.
 * @status partial
 * @match 98.73
 * @residual Extra unsigned-byte mask after the angle mask; four bytes over.
 */
void seedRandomRadiusAndProjectWorkPositionPairThenAdvanceBmagicMagic09303_801EF4C8(void) {
  u8 *initial;
  u8 *record;
  u8 *work;
  s16 *radius;
  s16 angle;
  s32 sample;

  g_battle_work[0xA] = (rand() & 3) << 4;
  initial = g_battle_work;
  radius = &D_1F800000;
  *radius = 0xC0 - initial[0xA];
  angle = initial[4] & 7;
  D_1F80000A = angle << 9;
  sample = func_801782FC(angle << 9);
  *(s32 *)(g_battle_work + 0x34) =
      *(s32 *)(D_801EB4E0 + 0x34) + ((sample * *radius) >> 3);
  sample = func_801783C8(D_1F80000A);
  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x38) =
      *(s32 *)(record + 0x38) + ((sample * *radius) >> 3);
  *(s32 *)(work + 0x3C) = *(s32 *)(record + 0x3C);
  g_battle_work[0xB] = rand();
  g_battle_work[9] = 0;
  g_battle_work[2]++;
}
