#include "bof3/world/area03004_internal.h"

/* @source 0x801DECDC
 * @behavior AREA030 work-record handler 3 of the overlay handler table
 * 0x801E2314. While the record countdown byte 0x09 is clear it asks the shared
 * entry gate func_801E0680 first and, when that reports no match, stores 1 in
 * record byte 0x01 and leaves through the shared step pair. Otherwise it rolls
 * rand() once and maps the difference between the shared reference position
 * 0x80143FFC / 0x80144000 and the record x 0x34 / y 0x38 onto the eight signed
 * octants, storing 0..7 in the record route index byte 0x08 and storing
 * nothing when both differences are zero. It then takes the difference between
 * the shared byte 0x80143FCF and the 16-byte direction row at +0x14 of the
 * 0x801E2AB8 scale record selected by record byte 0x06, indexed by the low
 * nibble of D_801E3210[0x0E], clamps its signed byte to -1..3 and dispatches
 * the roll-driven route nudge on that class: class 0 flips bit 2 of the route
 * index and then, while roll bit 0 is set, moves it by 1, classes 1 and 2 move
 * it by the roll low two bits while roll bits 5-7 are clear (class 1) or roll
 * bits 6-7 are clear (class 2), class 3 moves it by 1 under the class 2 gate
 * and class 4 leaves it untouched, each move taken up when roll bit 3 is set,
 * down otherwise, and masked to the eight routes. It then re-derives the steps
 * through func_801E03F8, sets the countdown step word 0x14 from the
 * func_8015477C distance of the record x/y advanced by the step words
 * 0x0C/0x10 shifted 4 - the distance less the record timer 0x3E divided by 16
 * while that timer sits below the distance, else 2 or -2 from the shared byte
 * 0x80144006 against the same timer, else zero - mirrors the step-mode row
 * trailing byte into record byte 0x2A and requests the row leading byte
 * through func_8014D8D4. When func_801E0770 reports the record inside the
 * shared window and either the roll low nibble is under the scale record byte
 * 0x1C or bit 7 of the shared 0x801E3210 flag byte is set, it keeps going
 * unless that direction row byte reads 9: it saves record byte 0x05 into
 * 0x801E3208, arms the phase bytes 0x80145E92 = 4 / 0x80145E93 = 0 /
 * 0x80143FC9 = 5, clears the shared words 0x80143FD4 / 0x80143FD8 /
 * 0x80143FDC together with record byte 0x09, words 0x0C/0x10/0x14 and bytes
 * 0x0A/0x0B, increments record byte 0x01, sets the route index to 1, mirrors
 * and requests that row again and leaves through func_8014D978 without the
 * shared step pair. The remaining paths store 0x10 in record byte 0x09 and,
 * when the signed direction class reads 3, double the record words 0x0C, 0x14
 * and 0x10 once the roll low two bits are clear, then run the shared step pair
 * func_801E0394 / func_8014D978.
 * @status partial
 * @match 78.68
 * @residual 321/408 instructions with an 8-byte body surplus. Statements,
 * offsets, literals, branch targets and the switch topology match; the residue
 * is scheduling and allocation only, in four clusters. The seven octant arm
 * stores: the original materializes each stored constant before the volatile
 * cursor load (`li v0,N` in the branch delay slot, every arm storing inline in
 * its own jump delay slot) where this body loads the cursor first and the seven
 * stores then merge into one shared `sb`. The four roll-nudge arms swap the
 * value and cursor registers ($v0/$v1 against $a0/$v1) while keeping the same
 * instructions. The clamp merges its three phi arms into $s1 directly where the
 * original routes them through $v1 and copies once at the join. The accept
 * block and the doubling triple schedule one cursor reload one slot apart. The
 * switch jump table is unplaced: the original five-entry table sits at
 * 0x801D0C1C in the overlay head rodata, so the emitted lui/lw pair cannot match
 * until a `[[matching.section_placements]]` entry for that span validates, and
 * that validation compares the linked table bytes against the original, which
 * needs the case labels on their original offsets first.
 */
void steerRouteIndexTowardSharedReferenceAndArmEntryTransitionWorld00Area03004_801DECDC(
    void) {
  u8* work;
  u8* flip;
  u8* upOne;
  u8* downOne;
  u8* upRoll;
  u8* downRoll;
  u8* point;
  u8* rateWork;
  u8* marker;
  u8* arm;
  u8* staged;
  u8* bump;
  u8* armed;
  u8* doubled;
  u8* scaled;
  u8* dirRow;
  u8* rowPtr;
  u8 amount;
  u8 gate;
  u8 mode;
  s32 dx;
  s32 dy;
  s32 diff;
  s16 dist;
  s16 counter;
  u32 roll;

  if (D_1F800044[9] == 0) {
    if (func_801E0680() == 0) {
      D_1F800044[1] = 1;
    } else {
      roll = rand();
      work = D_1F800044;
      dx = D_80143FFC - *(s32*)(work + 0x34);
      dy = D_80144000 - *(s32*)(work + 0x38);
      if (dx < 0 && dy < 0) {
        work[8] = 0;
      } else if (dx == 0 && dy < 0) {
        D_1F800044[8] = 1;
      } else if (dx > 0 && dy < 0) {
        D_1F800044[8] = 2;
      } else if (dx > 0 && dy == 0) {
        D_1F800044[8] = 3;
      } else if (dx > 0 && dy > 0) {
        D_1F800044[8] = 4;
      } else if (dx == 0 && dy > 0) {
        D_1F800044[8] = 5;
      } else if (dx < 0 && dy > 0) {
        D_1F800044[8] = 6;
      } else if (dx < 0 && dy == 0) {
        D_1F800044[8] = 7;
      }
      dirRow = (u8*)&D_801E2AB8[D_1F800044[6]] + 0x14;
      diff = (u8)D_80143FCF - dirRow[D_801E3210[0x0E] & 0xF];
      if ((s8)diff >= -1) {
        if ((s8)diff > 3) {
          diff = 3;
        }
      } else {
        diff = -1;
      }
      switch ((s8)(diff + 1)) {
      case 0:
        flip = D_1F800044;
        flip[8] = flip[8] ^ 4;
        if (roll & 1) {
          if (roll & 2) {
            upOne = D_1F800044;
            upOne[8] = (u8)((upOne[8] + 1) & 7);
          } else {
            downOne = D_1F800044;
            downOne[8] = (u8)((downOne[8] - 1) & 7);
          }
        }
        break;
      case 1:
        if (roll & 0xE0) {
          if (roll & 8) {
            upRoll = D_1F800044;
            amount = (u8)(roll & 3);
            upRoll[8] = (u8)((upRoll[8] + amount) & 7);
          } else {
            downRoll = D_1F800044;
            amount = (u8)(roll & 3);
            downRoll[8] = (u8)((downRoll[8] - amount) & 7);
          }
        }
        break;
      case 2:
        if ((roll & 0xC0) == 0) {
          if (roll & 8) {
            upRoll = D_1F800044;
            amount = (u8)(roll & 3);
            upRoll[8] = (u8)((upRoll[8] + amount) & 7);
          } else {
            downRoll = D_1F800044;
            amount = (u8)(roll & 3);
            downRoll[8] = (u8)((downRoll[8] - amount) & 7);
          }
        }
        break;
      case 3:
        if ((roll & 0xC0) == 0) {
          if (roll & 8) {
            upOne = D_1F800044;
            upOne[8] = (u8)((upOne[8] + 1) & 7);
          } else {
            downOne = D_1F800044;
            downOne[8] = (u8)((downOne[8] - 1) & 7);
          }
        }
        break;
      case 4:
        break;
      }
      func_801E03F8();
      point = D_1F800044;
      dist = func_8015477C(*(s32*)(point + 0x34) + (*(s32*)(point + 0x0C) << 4),
                           *(s32*)(point + 0x38) + (*(s32*)(point + 0x10) << 4));
      rateWork = D_1F800044;
      counter = *(s16*)(rateWork + 0x3E);
      if (counter < dist) {
        *(s32*)(rateWork + 0x14) = (dist - counter) / 16;
      } else {
        *(s32*)(rateWork + 0x14) = 0;
        if (D_80144006 < *(s16*)(rateWork + 0x3E)) {
          *(s32*)(rateWork + 0x14) = -2;
        } else if (*(s16*)(rateWork + 0x3E) < D_80144006) {
          *(s32*)(rateWork + 0x14) = 2;
        }
      }
      marker = D_1F800044;
      marker[0x2A] = D_801E2330[marker[8]].byte_01;
      arm = D_1F800044;
      func_8014D8D4(D_801E2330[arm[8]].request_00);
      if (func_801E0770() != 0) {
        gate = D_801E2AB8[D_1F800044[6]].unk_1C[0];
        if ((rand() & 0xF) < gate || (D_801E3210[0x0E] & 0x80) != 0) {
          staged = D_1F800044;
          rowPtr = (u8*)&D_801E2AB8[staged[6]] + 0x14;
          if ((s8)rowPtr[D_801E3210[0x0E] & 0xF] != 9) {
            mode = staged[5];
            D_80145E92 = 4;
            D_80145E93 = 0;
            D_80143FC9 = 5;
            D_80143FD4 = 0;
            D_80143FD8 = 0;
            D_80143FDC = 0;
            D_801E3208 = mode;
            staged[9] = 0;
            *(s32*)(staged + 0x0C) = 0;
            *(s32*)(staged + 0x10) = 0;
            *(s32*)(staged + 0x14) = 0;
            D_1F800044[0x0A] = 0;
            D_1F800044[0x0B] = 0;
            bump = D_1F800044;
            bump[1] = (u8)(bump[1] + 1);
            D_1F800044[8] = 1;
            armed = D_1F800044;
            armed[0x2A] = D_801E2330[armed[8]].byte_01;
            doubled = D_1F800044;
            func_8014D8D4(D_801E2330[doubled[8]].request_00);
            func_8014D978();
            return;
          }
        }
      }
      D_1F800044[9] = 0x10;
      if ((s8)diff == 3) {
        if ((rand() & 3) == 0) {
          scaled = D_1F800044;
          *(s32*)(scaled + 0x0C) = *(s32*)(scaled + 0x0C) << 1;
          *(s32*)(scaled + 0x10) = *(s32*)(scaled + 0x10) << 1;
          *(s32*)(scaled + 0x14) = *(s32*)(scaled + 0x14) << 1;
          scaled[9] = 8;
        }
      }
    }
  }
  func_801E0394();
  func_8014D978();
}
