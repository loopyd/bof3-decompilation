#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DE018
 * @behavior Refreshes the AREA030 message state through func_801DDFD4, then
 * walks the four text slots: each slot's current character is read through its
 * D_801E28F4 record pointer at the D_801E31E0 counter and classified with the
 * D_801E31F0 state byte and the D_801E31E4 slot timer into an action 0/1/2;
 * action 1 advances the counter with a 12-frame timer reload while the current
 * character has no 0x80 terminator bit, otherwise it publishes the slot index
 * D_801E28F8[offset] plus the record byte 0x0F into the message state at
 * D_80143FCF once that sum reaches the signed limit there (also storing the
 * record byte 0x10 at +3) and resets the slot; action 2 resets the slot pair.
 * @status partial
 * @match 98.11
 * @residual the limit pointer materialization (lui/addiu $t4 for 0x80143FCF) is emitted at the end of the prologue instead of directly after the li $t5,1 state constant; all other 104 instructions, including the count->$t1 / record-pointer->$t0 allocation, match
 */
void func_801DE018(void) {
  s32 one;
  s8 *limit;
  u8 *count;
  u8 *timer;
  Area030TextSlot *slot;
  s32 offset;

  func_801DDFD4();

  one = 1;
  count = D_801E31E0;
  timer = D_801E31E4;
  slot = D_801E28F4;
  offset = 0;
  limit = &D_80143FCF;

  do {
    s32 ch;
    s32 masked;
    u8 state;
    s32 result;

    ch = slot->text[count[0]];
    result = 0;
    masked = ch & 0x7F;
    if (count[0] != 0) {
      timer[0] = timer[0] - 1;
    }
    state = D_801E31F0;
    if (state == one) {
      if (count[0] == 0) {
        result = 1;
      } else if (timer[0] < 4) {
        if (masked == one) {
          result = 1;
        } else {
          result = 2;
        }
      } else {
        result = 2;
      }
    } else if ((state == 0) && (timer[0] == 0)) {
      if (masked == 0) {
        result = 1;
      } else {
        result = 2;
      }
    }
    if (result == one) {
      if ((slot->text[count[0]] & 0x80) != 0) {
        u8 *rec = D_801E3210;

        if ((D_801E28F8[offset] + *(s8 *)(rec + 0xF)) >= limit[0]) {
          limit[0] = D_801E28F8[offset] + *(s8 *)(rec + 0xF);
          limit[3] = rec[0x10];
        }
        count[0] = 0;
        timer[0] = 0;
      } else {
        count[0] = count[0] + 1;
        timer[0] = 12;
      }
    } else if (result == 2) {
      timer[0] = 0;
      count[0] = 0;
    }
    count++;
    timer++;
    slot++;
    offset += 8;
  } while ((s32)count < (s32)D_801E31E4);
}
