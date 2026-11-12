#include "bof3/world/area03004_internal.h"

/* @source 0x801DE82C
 * @behavior AREA030 work-record handler 1 of the 0x801E2314 table. While the
 * record step countdown byte 0x09 is non-zero it only integrates that step
 * through func_801E0394 and runs func_8014D978, and the same pair closes every
 * other path except the entry request below. Otherwise it first consults the
 * shared entry gate func_801E0680 and, when that reports a match, sets the
 * work-record handler byte 0x01 to 3 and leaves through that pair. With the gate
 * clear it rolls rand() once and reuses the single roll. While the entry latch
 * D_801E3200 is clear, the record signed timer 0x3E is zero, the roll low two
 * bits are clear, the phase byte D_80145E92 is not 4 and record byte 0x06 is not
 * 0x16, it arms the latch, loads the entry halfword 0x12 of the D_801E2AB8 scale
 * record selected by record byte 0x06 through func_8014DD3C, queues cue 0x206
 * unless record flag 0x80 is set, clears record flag bit 5 and record bytes
 * 0x2A/0x48, releases the entry through func_8014D6B8(0), advances handler byte
 * 0x01 and returns without the integration pair. Otherwise it steps the route
 * index byte 0x08 to the neighbouring route by one modulo 8 (down when roll bit
 * 8 is clear, up when it is set) whenever roll bit 4 is set, re-derives the step
 * words through func_801E03F8, mirrors the step-mode row trailing byte into
 * record byte 0x2A and selects that row leading byte through func_8014D8D4. It
 * then sets the countdown step word 0x14 from the func_8015477C distance of the
 * record x at 0x34 and y at 0x38 advanced by the step words 0x0C/0x10 shifted 4:
 * the distance less the timer divided by 16 while the timer sits below that
 * distance, else zero, and then, while roll bit 0x10 is set, 2 when roll bit
 * 0x20 is also set and the timer plus 0x20 has reached the negated scale byte
 * 0x1A times 256, or -2 when roll bit 0x20 is clear and the timer less 0x20
 * holds at or above both the distance and the negated scale byte plus one times
 * 256. Roll bit 0x40 clears the step words 0x0C, 0x10 and 0x14, and the
 * countdown byte 0x09 restarts at 0x10 before the integrating pair. The
 * scratchpad cursor at 0x1F800044 is reloaded for every group of record
 * accesses.
 * @status partial
 * @match 95.22
 * @residual 219/230 instructions with an equal 920-byte body. The eleven
 * differing words are one allocation decision with two visible clusters and no
 * value difference: the original build homes the rand roll in $v1 with a caller
 * save to $s0 (`move v1,v0`, then `move s0,v1` in the D_801E3200 branch delay
 * slot) and tests the low two bits through $v1, where this body homes the roll
 * in $s0 directly and tests it there. That leaves the D_801E3200 byte in $v0
 * there and in $v1 here, which in turn denies the func_801E0680 branch delay
 * slot its `li v0,3` fill and moves that block's three leading words. The gate
 * branch (seven words) and the roll group (five words) close together or not at
 * all. Statements, literals, offsets, branch targets and the shared 0x14 store
 * block match word for word.
 * One cursor variable per scratchpad reload point is measured, not stylistic:
 * collapsing the eleven reload variables into one pointer drops the body to
 * 158/230, and the struct view of the record changes nothing.
 */
void runWorkRecordStepOrRequestEntryElseAdvanceRouteIndexRandomlyAndSeedCountdownStepWorld00Area03004_801DE82C(
    void) {
  u8* timer;
  u8* gateWork;
  u8* flags;
  u8* marker;
  u8* entry;
  u8* phase;
  u8* route;
  u8* row;
  u8* point;
  u8* rateWork;
  u8* clearWork;
  s16 distance;
  s16 counter;
  u32 roll;

  timer = D_1F800044;
  if (timer[9] == 0) {
    if (func_801E0680()) {
      D_1F800044[1] = 3;
    } else {
      roll = rand();
      if (D_801E3200 == 0) {
        gateWork = D_1F800044;
        if (*(s16*)(gateWork + 0x3E) == 0 && (roll & 3) == 0 &&
            D_80145E92 != 4 && gateWork[6] != 0x16) {
          D_801E3200 = 1;
          func_8014DD3C(D_801E2AB8[gateWork[6]].entry_12);
          if ((D_1F800044[0] & 0x80) == 0) {
            func_8015DF18(0x206);
          }
          flags = D_1F800044;
          flags[0] = (u8)(flags[0] & 0xDF);
          marker = D_1F800044;
          marker[0x2A] = 0;
          entry = D_1F800044;
          entry[0x48] = 0;
          func_8014D6B8(0);
          phase = D_1F800044;
          phase[1] = (u8)(phase[1] + 1u);
          return;
        }
      }
      if (roll & 4) {
        if (roll & 8) {
          route = D_1F800044;
          route[8] = (u8)((route[8] + 1u) & 7u);
        } else {
          route = D_1F800044;
          route[8] = (u8)((route[8] - 1u) & 7u);
        }
      }
      func_801E03F8();
      marker = D_1F800044;
      marker[0x2A] = D_801E2330[marker[8]].byte_01;
      row = D_1F800044;
      func_8014D8D4(D_801E2330[row[8]].request_00);
      point = D_1F800044;
      distance = func_8015477C(
          *(s32*)(point + 0x34) + (*(s32*)(point + 0x0C) << 4),
          *(s32*)(point + 0x38) + (*(s32*)(point + 0x10) << 4));
      rateWork = D_1F800044;
      counter = *(s16*)(rateWork + 0x3E);
      if (counter < distance) {
        *(s32*)(rateWork + 0x14) = (distance - counter) / 16;
      } else {
        *(s32*)(rateWork + 0x14) = 0;
        if (roll & 0x10) {
          if (roll & 0x20) {
            counter = *(s16*)(rateWork + 0x3E);
            if (counter != 0 &&
                -(((s32)D_801E2AB8[rateWork[6]].unk_1A) << 8) >=
                    counter + 0x20) {
              *(s32*)(rateWork + 0x14) = 2;
            }
          } else {
            counter = *(s16*)(rateWork + 0x3E);
            if (counter - 0x20 >= distance &&
                counter - 0x20 >=
                    -((((s32)D_801E2AB8[rateWork[6]].unk_1A) + 1) << 8)) {
              *(s32*)(rateWork + 0x14) = -2;
            }
          }
        }
      }
      if (roll & 0x40) {
        clearWork = D_1F800044;
        *(s32*)(clearWork + 0x0C) = 0;
        *(s32*)(clearWork + 0x10) = 0;
        *(s32*)(clearWork + 0x14) = 0;
      }
      D_1F800044[9] = 0x10;
    }
  }
  func_801E0394();
  func_8014D978();
}
