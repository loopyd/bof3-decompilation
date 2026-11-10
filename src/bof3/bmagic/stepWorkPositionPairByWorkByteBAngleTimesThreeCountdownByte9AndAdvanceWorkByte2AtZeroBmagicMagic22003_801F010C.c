#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 func_801782FC(s32 angle);
extern s32 func_801783C8(s32 angle);

/* @source 0x801F010C
 * @behavior Handler referenced by the table word at 0x801F0708. Seeds the
 * scratchpad halfword at 0x1F800004 from work byte 0xB shifted left eight,
 * passes the signed halfword angle to 0x801782FC, and adds three times its
 * answer to work word 0x34. Reads the scratchpad angle for 0x801783C8 and
 * adds three times that answer to work word 0x38. Decrements work byte 9
 * at byte width and advances work byte 2 when the stored countdown is zero.
 * Each call is followed by a fresh work-pointer read; the zero-count arm
 * reads the pointer again. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairByWorkByteBAngleTimesThreeCountdownByte9AndAdvanceWorkByte2AtZeroBmagicMagic22003_801F010C(void) {
  s16 *cell;
  s32 angle;
  s32 sample;
  s32 value;
  u8 *work;
  u8 *work2;
  u8 count;

  angle = g_battle_work[0xB] * 0x100;
  cell = &D_1F800004;
  *cell = angle;
  sample = func_801782FC((s16)angle);
  work = g_battle_work;
  *(s32 *)(work + 0x34) += sample * 3;
  sample = func_801783C8(*cell);
  work2 = g_battle_work;
  count = (u8)(work2[9] - 1);
  value = *(s32 *)(work2 + 0x38) + sample * 3;
  work2[9] = count;
  *(s32 *)(work2 + 0x38) = value;
  if (count == 0) {
    g_battle_work[2]++;
  }
}
