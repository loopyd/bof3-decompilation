#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s32 func_801782FC(s32 angle);
extern s32 func_801783C8(s32 angle);

void clearWorkBytesBmagicMagic22003_801F068C(void);

/* @source 0x801F01C8
 * @behavior Entry 12 (word at 0x801F070C) of the handler table based at
 * 0x801F06DC, beside the exact sibling at 0x801F010C. Seeds the scratchpad
 * halfword at 0x1F800004 from work byte 0xB shifted left eight, passes the
 * signed halfword angle to 0x801782FC, and adds twice its answer to work word
 * 0x34. Reads the scratchpad angle for 0x801783C8 and adds twice that answer
 * to work word 0x38. Decrements work byte 0xA by two at byte width and, when
 * the stored countdown reaches zero, decrements the shared battle work byte
 * 0xB behind 0x801EB4E0 and invokes this overlay's work-byte clear routine at
 * 0x801F068C, the store filling the call delay slot. Each call is followed by
 * a fresh work-pointer read. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairByWorkByteBAngleTimesTwoCountdownByteAByTwoThenDecrementGlobalWorkByteBAndClearWorkAtZeroBmagicMagic22003_801F01C8(void) {
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
  *(s32 *)(work + 0x34) += sample * 2;
  sample = func_801783C8(*cell);
  work2 = g_battle_work;
  count = (u8)(work2[0xA] - 2);
  value = *(s32 *)(work2 + 0x38) + sample * 2;
  work2[0xA] = count;
  *(s32 *)(work2 + 0x38) = value;
  if (count == 0) {
    D_801EB4E0[0xB] -= 1;
    clearWorkBytesBmagicMagic22003_801F068C();
  }
}
