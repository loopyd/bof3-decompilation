#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 front-end entry-record motion step, handler index 1 of the
 * 0x801E2258 table dispatched through work-record byte 3. It first retires the
 * scratch record byte 6 through
 * storeRequestedStateAndAdvanceSelectedModesWorld00Area03004(1) once the
 * shared selection gate D_80144199 holds 3; while the pad state D_80145AA8
 * carries Up and that gate is 0 it queues cue 0x203, advances scratch record
 * byte 3, clears D_801440B1 and D_80143FC9 and stores 7 in modeByte; while the
 * shared word D_80144000 exceeds 0x3E7FFF it queues the same cue, clears those
 * two bytes, stores 1 in scratch record byte 2 and clears byte 3. Otherwise,
 * once the map height sampled at the shared coordinates D_80143FFC/D_80144000
 * reaches the signed halfword D_80144006 and both the selected entry byte 0x11
 * and the frame-counter nibble of D_80143E6C pass their random gate, it clears
 * the entry record words 0x0C/0x10/0x14, lifts D_8014930C to D_80144000 when
 * that word trails it, stores 0x10 in D_8014932E, sets scratch record byte 2
 * to 5 and clears byte 3; a failed random gate instead republishes the map
 * height sampled at the record coordinates into record halfword 0x3E. It then
 * runs the hold-marker step
 * updateScratchMarkerAndCountdownFromPadFlagsWorld00Area03004, the message
 * step updateFourTextSlotsAndPublishCompletedSlotLimitWorld00Area03004 and
 * publishes the selected value D_801E31F8 into D_801E31FC; while entry record
 * byte 0x0B is clear it applies the pad direction to the record motion triple
 * 0x0C/0x10/0x14 and the selected value D_801E31F8 - Square (0x8000) with the
 * third word from selected entry word 8 and request 6, Circle (0x2000) with
 * the same word and request 5, Down (0x40) with the selection state 2/1 and
 * request 4/3 split by scratch byte 0 and the third word from selected entry
 * word 8/4, and no direction at all with request 4/3 split the same way -
 * passing that request to func_8014D8D4. Finally it plays the selection cue
 * playCueForChangedSelectedValueWorld00Area03004, publishes record word 0x38
 * into D_8014930C, derives D_8014932E from record word 0x10 (>> 8, or 0x10
 * when clear), clamps D_8014930C into 0x150000..0x3C0000, stops record word
 * 0x0C at the 0x80000/0x11FFFF x walls and runs func_8014D978 while record
 * word 0x10 stays positive.
 * @source 0x801DA9DC
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepEntryRecordMotionOnPadAndClampSharedChannelsWorld00Area03004_801DA9DC(void) {
  u8* row;
  s32 pick;
  s16 dist;
  s32 value;
  s32 selected;
  u32* motionChannel;
  u32* clampChannel;
  u8* entry;

  if (SPAD_PTR_SLOT(u8, 0x44u)[6] != 0) {
    if (D_80144199_BYTE == 3) {
      SPAD_PTR_SLOT(u8, 0x44u)[6] = 0;
      storeRequestedStateAndAdvanceSelectedModesWorld00Area03004(1);
    }
  }
  if ((D_80145AA8 & 0x10) != 0) {
    if (D_80144199_BYTE == 0) {
      func_8015DF18(0x203);
      SPAD_PTR_SLOT(u8, 0x44u)[3] = SPAD_PTR_SLOT(u8, 0x44u)[3] + 1;
      D_801440B1 = 0;
      D_80143FC9 = 0;
      modeByte = 7;
      return;
    }
  }
  row = (u8*)D_80143FC8;
  if (D_80144000 > 0x3E7FFF) {
    func_8015DF18(0x203);
    D_801440B1 = 0;
    D_80143FC9 = 0;
    D_1F800044[2] = 1;
    D_1F800044[3] = 0;
    return;
  }
  dist = sampleMapHeight(D_80143FFC, D_80144000);
  if (dist >= D_80144006) {
    pick = *(s8*)(D_801E3210 + 0x11);
    if ((rand() & 0xF) < pick) {
      if ((D_80143E6C & 0xF) == 0) {
        D_80143FD4 = 0;
        D_80143FD8 = 0;
        D_80143FDC = 0;
        motionChannel = &D_8014930C;
        if ((s32)*motionChannel < D_80144000) {
          *motionChannel = D_80144000;
        }
        D_8014932E = 0x10;
        D_1F800044[2] = 5;
        D_1F800044[3] = 0;
        return;
      }
    }
    *(s16*)(row + 0x3E) = sampleMapHeight(*(s32*)(row + 0x34), *(s32*)(row + 0x38));
  }
  updateScratchMarkerAndCountdownFromPadFlagsWorld00Area03004();
  updateFourTextSlotsAndPublishCompletedSlotLimitWorld00Area03004();
  D_801E31FC = D_801E31F8;
  if (row[0x0B] == 0) {
    if ((D_80145AA4 & 0x8000) != 0) {
      entry = D_801E3210;
      *(s32*)(row + 0x0C) = -0x1000;
      *(s32*)(row + 0x10) = 0x1000;
      *(s32*)(row + 0x14) = *(s32*)(entry + 8);
      D_801E31F8 = 1;
      func_8014D8D4(6);
    } else if ((D_80145AA4 & 0x2000) != 0) {
      *(s32*)(row + 0x0C) = 0x1000;
      *(s32*)(row + 0x10) = 0x1000;
      *(s32*)(row + 0x14) = *(s32*)(D_801E3210 + 8);
      D_801E31F8 = 1;
      func_8014D8D4(5);
    } else if ((D_80145AA4 & 0x40) != 0) {
      *(s32*)(row + 0x0C) = 0;
      if (SPAD_REF(u8, 0) != 0) {
        *(s32*)(row + 0x10) = 0x2000;
        *(s32*)(row + 0x14) = *(s32*)(D_801E3210 + 8);
        D_801E31F8 = 2;
        func_8014D8D4(4);
      } else {
        *(s32*)(row + 0x10) = 0x1000;
        *(s32*)(row + 0x14) = *(s32*)(D_801E3210 + 4);
        D_801E31F8 = 1;
        func_8014D8D4(3);
      }
    } else if (SPAD_REF(u8, 0) != 0) {
      entry = D_801E3210;
      *(s32*)(row + 0x0C) = 0;
      *(s32*)(row + 0x10) = 0x1000;
      *(s32*)(row + 0x14) = *(s32*)(entry + 8);
      D_801E31F8 = 2;
      func_8014D8D4(4);
    } else {
      *(s32*)(row + 0x0C) = 0;
      *(s32*)(row + 0x10) = 0;
      *(s32*)(row + 0x14) = *(s32*)(D_801E3210 + 0);
      D_801E31F8 = 0;
      func_8014D8D4(3);
    }
  }
  playCueForChangedSelectedValueWorld00Area03004();
  D_8014930C = *(s32*)(row + 0x38);
  if (*(s32*)(row + 0x10) != 0) {
    D_8014932E = *(s32*)(row + 0x10) >> 8;
  } else {
    D_8014932E = 0x10;
  }
  clampChannel = &D_8014930C;
  value = *clampChannel;
  if (value > 0x14FFFF) {
    if (value <= 0x3C0000) {
      selected = value;
    } else {
      selected = 0x3C0000;
    }
  } else {
    selected = 0x150000;
  }
  *clampChannel = selected;
  if ((*(s32*)(row + 0x34) <= 0x80000 && *(s32*)(row + 0x0C) < 0) ||
      (*(s32*)(row + 0x34) > 0x11FFFF && *(s32*)(row + 0x0C) > 0)) {
    *(s32*)(row + 0x0C) = 0;
  }
  if (*(s32*)(row + 0x10) > 0) {
    func_8014D978();
  }
}
