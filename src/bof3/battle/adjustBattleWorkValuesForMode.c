#include "bof3/battle/battle15_internal.h"

/* @behavior Adjusts battle work fields according to mode values 1 through 4.
 * Cases 2 and 3 scan counted slots and skip unavailable battlers. The gameplay
 * meanings of the mode values and adjusted fields are not established.
 * @source 0x800AAA74
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustBattleWorkValuesForMode(void) {
  switch (battleAdjustmentModeState) {
  case 1u: {
    u8* workBase;

    workBase = battleWorkAdjustmentFields;
    adjustHalfwordWith999Cap((volatile u16*)workBase, (u32)(*(volatile u16*)workBase) >> 1u);
    adjustHalfwordWith999Cap((volatile u16*)(workBase + 2),
                  -(s32)((u32)(*(volatile u16*)(workBase + 2)) >> 2u));
    adjustByteWith100Cap(workBase + 21, *(volatile u8*)(workBase + 21));
    return;
  }
  case 2u: {
    u8  battlerSlot;
    u32 recordOffset;

    battlerSlot = 0u;
    while (battlerSlot < localBattlerCount) {
      if (isBattlerInactiveOrFlag4000(battlerSlot) == 0u) {
        volatile u16* recordBase;

        recordOffset = (u32)battlerSlot * 0x140u;
        recordBase = (volatile u16*)(battleWorkRecordBase + recordOffset);
        adjustHalfwordWith999Cap(recordBase + 0x22,
                      -(s32)((u32)(*(volatile u16*)(battleWorkRecordBase + recordOffset + 0x44u)) >> 2u));
      }
      battlerSlot += 1u;
    }
    return;
  }
  case 3u: {
    u8  battlerSlot;
    u32 recordOffset;

    battlerSlot = 0u;
    while (battlerSlot < localBattlerCount) {
      if (isBattlerInactiveOrFlag4000(battlerSlot) == 0u) {
        u32 halfValue;
        volatile u16 *valueAddress;
        s32 addressOrDelta;

        recordOffset = (u32)battlerSlot * 0x140u;
        halfValue = *(volatile u16*)(battleWorkRecordBase + recordOffset + 0x42u);
        halfValue >>= 1u;
        *(volatile u16*)(battleWorkRecordBase + recordOffset + 0x44u) =
            *(volatile u16*)(battleWorkRecordBase + 0x44u);
        addressOrDelta = (s32)recordOffset + (s32)battleWorkRecordBase;
        addressOrDelta += 0x42;
        valueAddress = (volatile u16*)addressOrDelta;
        do {
          addressOrDelta = -(s32)halfValue;
        } while (0);
        adjustHalfwordWith999Cap(valueAddress, addressOrDelta);
      }
      battlerSlot += 1u;
    }
    break;
  }
  case 4u: {
    u8* workBase;

    workBase = battleCalculationInputAdjustmentState;
    adjustHalfwordWith999Cap((volatile u16*)workBase, (u32)(*(volatile u16*)workBase) >> 1u);
    adjustHalfwordWith999Cap((volatile u16*)(workBase - 0x280),
                  -(s32)((u32)(*(volatile u16*)(workBase - 0x280)) >> 2u));
    adjustHalfwordWith999Cap((volatile u16*)(workBase - 0x140),
                  -(s32)((u32)(*(volatile u16*)(workBase - 0x140)) >> 2u));
    break;
  }
  }
}
