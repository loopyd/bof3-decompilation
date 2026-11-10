#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

extern s32 func_801782FC(s32 angle);
extern s32 func_801783C8(s32 angle);

/* @source 0x801EF600
 * @behavior Project work words 0x34 and 0x38 from the corresponding words
 *           of the record at 0x801EB4E0 using the two angular samples at
 *           (work byte 4 & 7) << 9. Publish that angle to scratchpad halfword
 *           0x1F80000A and the signed radius (byte 9 << 3) - (byte A - 0xC0)
 *           to scratchpad halfword 0x1F800000. Each sample times the re-read
 *           radius, shifted right three, offsets its corresponding word.
 *           Increment byte 9 at byte width before storing word 0x38, then
 *           advance byte 2 when the incremented byte equals 16. Re-read the
 *           scratchpad work pointer at each access group. The entry address
 *           occurs at table word 0x801F08B8, with no direct in-payload call.
 *           Takes no arguments and returns nothing; field roles remain
 *           offset-anchored.
 * @status partial
 * @match 97.06
 * @residual Angle mask and duplicated shift scheduling differ.
 */
void projectWorkPositionPairFromByte9AndByteAAtByte4AngleThenAdvanceAt16BmagicMagic09303_801EF600(void) {
  u8 *initial;
  u8 *record;
  u8 *work;
  s16 *radius;
  s32 angle;
  s32 sample;
  s32 position;
  u8 count;

  initial = g_battle_work;
  radius = &D_1F800000;
  *radius = (initial[9] << 3) - (initial[0xA] - 0xC0);
  angle = initial[4];
  D_1F80000A = (angle & 7) << 9;
  sample = func_801782FC((angle << 9) & 0xE00);
  *(s32 *)(g_battle_work + 0x34) =
      *(s32 *)(D_801EB4E0 + 0x34) + ((sample * *radius) >> 3);
  sample = func_801783C8(D_1F80000A);
  work = g_battle_work;
  record = D_801EB4E0;
  position = *(s32 *)(record + 0x38) + ((sample * *radius) >> 3);
  count = ++work[9];
  *(s32 *)(work + 0x38) = position;
  if (count == 16) {
    g_battle_work[2]++;
  }
}
