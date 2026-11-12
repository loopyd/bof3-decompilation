#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 front-end entry-record motion step, handler index 1 of the
 * 0x801E2258 table dispatched through work-record byte 3. It first retires the
 * scratch record byte 6 through func_801DDE94(1) once the shared selection gate
 * D_80144199 holds 3; while the pad state D_80145AA8 carries Up and that gate is
 * 0 it queues cue 0x203, advances scratch record byte 3, clears D_801440B1 and
 * D_80143FC9 and stores 7 in modeByte; while the shared word D_80144000 exceeds
 * 0x3E7FFF it queues the same cue, clears those two bytes, stores 1 in scratch
 * record byte 2 and clears byte 3. Otherwise, once the distance of the shared
 * pair D_80143FFC/D_80144000 reaches the signed halfword D_80144006 and both the
 * selected entry byte 0x11 and the frame-counter nibble of D_80143E6C pass their
 * random gate, it clears the entry record words 0x0C/0x10/0x14, lifts
 * D_8014930C to D_80144000 when that word trails it, stores 0x10 in D_8014932E,
 * sets scratch record byte 2 to 5 and clears byte 3; a failed random gate
 * instead republishes the record pair distance into record halfword 0x3E. It
 * then runs the hold-marker step func_801DDEEC, the message step func_801DE018
 * and publishes the selected value D_801E31F8 into D_801E31FC; while entry record
 * byte 0x0B is clear it applies the pad direction to the record motion triple
 * 0x0C/0x10/0x14 and the selected value D_801E31F8 - Square (0x8000) with the
 * third word from selected entry word 8 and request 6, Circle (0x2000) with the
 * same word and request 5, Down (0x40) with the selection state 2/1 and request
 * 4/3 split by scratch byte 0 and the third word from selected entry word 8/4,
 * and no direction at all with request 4/3 split the same way - passing that
 * request to func_8014D8D4. Finally it plays the selection cue func_801DDD4C,
 * publishes record word 0x38 into D_8014930C, derives D_8014932E from record
 * word 0x10 (>> 8, or 0x10 when clear), clamps D_8014930C into
 * 0x150000..0x3C0000, stops record word 0x0C at the 0x80000/0x11FFFF x walls and
 * runs func_8014D978 while record word 0x10 stays positive.
 * @source 0x801DA9DC
 * @status partial
 * @match 89.64
 * @residual same-size (1120/1120, 251/280 instructions). Five clusters remain,
 * all of address-form or scheduling class rather than of statement content. The
 * first is +0x00E4: the 0x3E7FFF arm keeps the scratch-cursor cell address in a
 * register and loads the cell after the two mode-byte stores, where the original
 * folds it into `lw v0,68(v0)` ahead of them. The others are the selected-entry
 * byte 0x11 read (measured `lbu` plus sign extension here against the original
 * `lb`), the D_8014930C read-modify-write in the random arm and again in the
 * clamp (the original keeps the cell address in $a0/$a1 and stores through it,
 * measured `lui at` absolute forms here), and the selected-entry load placement
 * inside the Square and no-direction arms. About 23 measured variants over six
 * shape sweeps spanned the cursor form (macro, volatile cell through one
 * variable, inline volatile, mixed), the pick width, the D_8014930C view (plain,
 * address expression, per-block pointer, function pointer, volatile cast), the
 * final-clamp shape (nested, three-way, value variable, pointer view) and the
 * pad-arm ordering; every one scored below this candidate. Next untried evidence
 * is an authorized compiler-profile probe or the signedness of the shared
 * D_8014930C declaration, which reaches other claimed spans.
 */
void stepEntryRecordMotionOnPadAndClampSharedChannelsWorld00Area03004_801DA9DC(void) {
  u8* row;
  s8 pick;
  s16 dist;
  s32 value;

  if (SPAD_PTR_SLOT(u8, 0x44u)[6] != 0) {
    if (D_80144199_BYTE == 3) {
      SPAD_PTR_SLOT(u8, 0x44u)[6] = 0;
      func_801DDE94(1);
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
    SPAD_PTR_SLOT(u8, 0x44u)[2] = 1;
    SPAD_PTR_SLOT(u8, 0x44u)[3] = 0;
    return;
  }
  dist = func_8015477C(D_80143FFC, D_80144000);
  if (dist >= D_80144006) {
    pick = *(s8*)(D_801E3210 + 0x11);
    if ((rand() & 0xF) < pick) {
      if ((D_80143E6C & 0xF) == 0) {
        D_80143FD4 = 0;
        D_80143FD8 = 0;
        D_80143FDC = 0;
        if ((s32)D_8014930C < D_80144000) {
          D_8014930C = D_80144000;
        }
        D_8014932E = 0x10;
        SPAD_PTR_SLOT(u8, 0x44u)[2] = 5;
        SPAD_PTR_SLOT(u8, 0x44u)[3] = 0;
        return;
      }
    }
    *(s16*)(row + 0x3E) = func_8015477C(*(s32*)(row + 0x34), *(s32*)(row + 0x38));
  }
  func_801DDEEC();
  func_801DE018();
  D_801E31FC = D_801E31F8;
  if (row[0x0B] == 0) {
    if ((D_80145AA4 & 0x8000) != 0) {
      *(s32*)(row + 0x0C) = -0x1000;
      *(s32*)(row + 0x10) = 0x1000;
      *(s32*)(row + 0x14) = *(s32*)(D_801E3210 + 8);
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
      *(s32*)(row + 0x0C) = 0;
      *(s32*)(row + 0x10) = 0x1000;
      *(s32*)(row + 0x14) = *(s32*)(D_801E3210 + 8);
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
  func_801DDD4C();
  D_8014930C = *(s32*)(row + 0x38);
  if (*(s32*)(row + 0x10) != 0) {
    D_8014932E = *(s32*)(row + 0x10) >> 8;
  } else {
    D_8014932E = 0x10;
  }
  value = D_8014930C;
  if (value > 0x14FFFF) {
    if (value > 0x3C0000) {
      value = 0x3C0000;
    }
  } else {
    value = 0x150000;
  }
  D_8014930C = value;
  if ((*(s32*)(row + 0x34) <= 0x80000 && *(s32*)(row + 0x0C) < 0) ||
      (*(s32*)(row + 0x34) > 0x11FFFF && *(s32*)(row + 0x0C) > 0)) {
    *(s32*)(row + 0x0C) = 0;
  }
  if (*(s32*)(row + 0x10) > 0) {
    func_8014D978();
  }
}
