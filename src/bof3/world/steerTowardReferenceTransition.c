#include "bof3/world/area03004_internal.h"

/* @source 0x801DECDC
 * @behavior AREA030 handler 3 of the work-record table at 0x801E2314. While countdown
 * byte 9 is clear, asks checkEntrySelectOne
 * for the entry gate. A clear answer sets work byte 1 to 1. Otherwise, samples
 * rand() and maps the shared reference position minus work coordinates
 * 0x34/0x38 onto the eight route octants in byte 8; zero deltas leave that
 * byte unchanged. Selects a direction byte from scale row D_801E2AB8[work[6]]
 * at +0x14, indexed by D_801E3210[0x0E] & 15. Subtracts that direction byte
 * from the unsigned shared byte D_80143FCF, classifies the signed low byte and
 * clamps that class to -1..3, keeping the full raw difference on the in-range
 * arm. For switch class 0, flips route bit 2 and nudges by 1 when roll bit 0
 * is set, with bit 1 choosing up or down. Class 1 nudges by roll & 3 when roll
 * & 0xE0 is nonzero; class 2 uses the same nudge when roll & 0xC0 is zero.
 * Class 3 nudges by 1 under the class 2 gate. Classes 1..3 choose up with roll
 * bit 3 and down otherwise. Class 4 leaves the route unchanged. Each nudge
 * wraps to eight routes. Recomputes steps through
 * deriveReflectStepsReindex and derives work
 * word 0x14 from the map height sampled at the advanced coordinates and the
 * signed counter at 0x3E. Mirrors the route's step-mode byte into work byte
 * 0x2A and requests its leading byte through func_8014D8D4. After
 * checkReferenceUnderGates succeeds, a fresh
 * random nibble below scale byte 0x1C or mode flag bit 7 permits entry when
 * the direction byte is not 9. That path saves work byte 5 to D_801E3208, arms
 * phase bytes D_80145E92/D_80145E93/D_80143FC9 as 4/0/5, clears shared motion
 * words D_80143FD4/D_80143FD8/D_80143FDC and work byte 9, words 0x0C/0x10/0x14
 * and bytes 0x0A/0x0B, increments work byte 1, selects route 1, requests that
 * route and returns through func_8014D978. Other active paths set countdown
 * byte 9 to 0x10; signed class 3 doubles the motion words and sets countdown 8
 * when a fresh rand() & 3 is zero. All paths except the accepted entry run
 * stateTripleWhileActive and func_8014D978.
 * @status exact
 * @match 100.00
 * @residual none
 */
void steerTowardReferenceTransition(
    void) {
  u8* work;
  u8* flip;
  u8* upOne;
  u8* downOne;
  u8* point;
  u8* rateWork;
  u8* marker;
  u8* arm;
  u8* staged;
  u8* clear_work;
  u8* bump;
  u8* armed;
  u8* doubled;
  u8* scaled;
  u8* dirRow;
  u8* direction_work;
  u32 scale_index;
  u32 row_offset;
  u32 direction_index;
  u8* rowPtr;
  u32 staged_direction;
  u32 staged_scale;
  u32 staged_offset;
  u32 amount;
  u32 route_value;
  u8 gate;
  u8 mode;
  s32 dx;
  s32 dy;
  s16 diff;
  s32 raw_class;
  s16 selected_class;
  s16 dist;
  s16 counter;
  u32 roll;

  if (D_1F800044[9] == 0) {
    if (checkEntrySelectOne() == 0) {
      SPAD_PTR_SLOT(u8, 0x44u)[1] = 1;
    } else {
      roll = rand();
      work = D_1F800044;
      dx = D_80143FFC - *(s32*)(work + 0x34);
      dy = D_80144000 - *(s32*)(work + 0x38);
      if (dx < 0 && dy < 0) {
        work[8] = 0;
      } else if (dx == 0 && dy < 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 1;
      } else if (dx > 0 && dy < 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 2;
      } else if (dx > 0 && dy == 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 3;
      } else if (dx > 0 && dy > 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 4;
      } else if (dx == 0 && dy > 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 5;
      } else if (dx < 0 && dy > 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 6;
      } else if (dx < 0 && dy == 0) {
        SPAD_PTR_SLOT(u8, 0x44u)[8] = 7;
      }
      do {
        direction_work = D_1F800044;
      } while (0);
      direction_index = (u32)D_801E3210;
      do {
        scale_index = direction_work[6];
      } while (0);
      direction_index = ((u8*)direction_index)[0x0E];
      do {
        row_offset = scale_index * sizeof(D_801E2AB8[0]);
      } while (0);
      direction_index &= 0xF;
      dirRow = (u8*)D_801E2AB8 + row_offset + 0x14;
      raw_class = (u8)D_80143FCF - dirRow[direction_index];
      diff = raw_class;
      raw_class = (u32)raw_class << 24;
      if ((raw_class >> 24) >= -1) {
        if ((raw_class >> 24) > 3) {
          selected_class = 3;
        } else {
          selected_class = diff;
        }
      } else {
        selected_class = -1;
      }
      diff = selected_class;
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
            u8* cursor;
            cursor = D_1F800044;
            amount = roll & 3;
            route_value = cursor[8];
            route_value = route_value + amount;
            route_value &= 7;
            cursor[8] = (u8)route_value;
          } else {
            u8* cursor;
            cursor = D_1F800044;
            amount = roll & 3;
            route_value = cursor[8];
            route_value = route_value - amount;
            route_value &= 7;
            cursor[8] = (u8)route_value;
          }
        }
        break;
      case 2:
        if ((roll & 0xC0) == 0) {
          if (roll & 8) {
            u8* cursor;
            cursor = D_1F800044;
            amount = roll & 3;
            route_value = cursor[8];
            route_value = route_value + amount;
            route_value &= 7;
            cursor[8] = (u8)route_value;
          } else {
            u8* cursor;
            cursor = D_1F800044;
            amount = roll & 3;
            route_value = cursor[8];
            route_value = route_value - amount;
            route_value &= 7;
            cursor[8] = (u8)route_value;
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
      deriveReflectStepsReindex();
      point = D_1F800044;
      dist = sampleMapHeight(*(s32*)(point + 0x34) + (*(s32*)(point + 0x0C) << 4),
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
      if (checkReferenceUnderGates() != 0) {
        gate = D_801E2AB8[D_1F800044[6]].unk_1C[0];
        if ((rand() & 0xF) < gate || (D_801E3210[0x0E] & 0x80) != 0) {
          do {
            staged = D_1F800044;
          } while (0);
          staged_direction = (u32)D_801E3210;
          do {
            staged_scale = staged[6];
          } while (0);
          staged_direction = ((u8*)staged_direction)[0x0E];
          do {
            staged_offset = staged_scale * sizeof(D_801E2AB8[0]);
          } while (0);
          staged_direction &= 0xF;
          rowPtr = (u8*)D_801E2AB8 + staged_offset + 0x14;
          if ((s8)rowPtr[staged_direction] != 9) {
            mode = staged[5];
            D_80145E92 = 4;
            D_80145E93 = 0;
            D_80143FC9 = 5;
            D_80143FD4 = 0;
            D_80143FD8 = 0;
            D_80143FDC = 0;
            D_801E3208 = mode;
            staged[9] = 0;
            clear_work = D_1F800044;
            *(s32*)(staged + 0x0C) = 0;
            *(s32*)(staged + 0x10) = 0;
            *(s32*)(staged + 0x14) = 0;
            clear_work[0x0A] = 0;
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
  stateTripleWhileActive();
  func_8014D978();
}
