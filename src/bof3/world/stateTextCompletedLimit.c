#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DE018
 * @behavior Refreshes the AREA030 message textAdvanceState through saveUpdateTextAdvance, then
 * walks the four text slots: each textSlot's current character is read through its
 * D_801E28F4 record pointer at the D_801E31E0 counter and classified with the
 * D_801E31F0 textAdvanceState byte and the D_801E31E4 textSlot slotTimer into an action 0/1/2;
 * action 1 advances the counter with a 12-frame slotTimer reload while the current
 * character has no 0x80 terminator bit, otherwise it publishes the textSlot index
 * D_801E28F8[slotByteOffset] plus the record byte 0x0F into the message textAdvanceState at
 * D_80143FCF once that sum reaches the signed publishedLimit there (also storing the
 * record byte 0x10 at +3) and resets the textSlot; action 2 resets the textSlot pair.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTextCompletedLimit(void) {
  s32 one;
  s8 *publishedLimit;
  u8 *slotCount;
  u8 *slotTimer;
  Area030TextSlot *textSlot;
  s32 slotByteOffset;

  saveUpdateTextAdvance();

  one = 1;
  publishedLimit = &D_80143FCF;
  slotCount = D_801E31E0;
  slotTimer = D_801E31E4;
  textSlot = D_801E28F4;
  slotByteOffset = 0;

  do {
    s32 character;
    s32 characterCode;
    u8 textAdvanceState;
    s32 slotAction;

    character = textSlot->text[slotCount[0]];
    slotAction = 0;
    characterCode = character & 0x7F;
    if (slotCount[0] != 0) {
      do {
        slotTimer[0] = slotTimer[0] - 1;
      } while (0);
    }
    textAdvanceState = D_801E31F0;
    if (textAdvanceState == one) {
      if (slotCount[0] == 0) {
        slotAction = 1;
      } else if (slotTimer[0] < 4) {
        if (characterCode == one) {
          slotAction = 1;
        } else {
          slotAction = 2;
        }
      } else {
        slotAction = 2;
      }
    } else if ((textAdvanceState == 0) && (slotTimer[0] == 0)) {
      if (characterCode == 0) {
        slotAction = 1;
      } else {
        slotAction = 2;
      }
    }
    if (slotAction == one) {
      if ((textSlot->text[slotCount[0]] & 0x80) != 0) {
        u8 *limitRecord;

        limitRecord = D_801E3210;

        if ((D_801E28F8[slotByteOffset] + *(s8 *)(limitRecord + 0xF)) >= publishedLimit[0]) {
          publishedLimit[0] = D_801E28F8[slotByteOffset] + *(s8 *)(limitRecord + 0xF);
          do {
            publishedLimit[3] = limitRecord[0x10];
          } while (0);
        }
        slotCount[0] = 0;
        slotTimer[0] = 0;
      } else {
        slotCount[0] = slotCount[0] + 1;
        slotTimer[0] = 12;
      }
    } else if (slotAction == 2) {
      slotTimer[0] = 0;
      slotCount[0] = 0;
    }
    slotCount++;
    do {
      slotTimer++;
    } while (0);
    textSlot++;
    slotByteOffset += 8;
  } while ((s32)slotCount < (s32)D_801E31E4);
}
