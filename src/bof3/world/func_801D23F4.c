#include "bof3/world/area03004_internal.h"

/* @behavior advances the AREA030 scroll record: clamps the work word at 0x10 into
 * 0x150000..0x3C0000 and publishes it to 0x8014930C. While that word still differs
 * from the target word at 0x38 it seeds the 0x20 accumulator from the 8-byte step
 * table entry selected by work byte 6 once the target matches 0x10 plus that entry's
 * first word, integrates 0x14 with 0x20, drops both accumulators when 0x10 plus the
 * entry word passes the target while 0x14 is negative, steps 0x34 and 0x3C by the
 * 0x0C and 0x14 words, lowers the target by 0x4000, arms halfword 0x8014932E with
 * 0x40 once the target reaches 0x3C0000 and calls func_8014D978. While the words are
 * equal it plays cue 513, clears work bytes 9 and 0x0A, the accumulators and the 0x3C
 * word, advances the work index and clears the 0x10 word unless the shared byte
 * 0x80149332 is set, requests resource 4 and falls through to func_8014D290.
 * @source 0x801D23F4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D23F4(void) {
  u8* cursor;
  u8* next;
  u8* state;
  u8* step;
  u8* flags;
  u8* serial;
  s32 value;
  s32 target;
  s32 limit;
  s32 delta;

  cursor = (u8*)D_1F800044;
  value = *(s32*)(cursor + 0x10u);
  if (value > 0x3BFFFF) {
    D_8014930C = 0x3C0000;
  } else if (value <= 0x14FFFF) {
    D_8014930C = 0x150000;
  } else {
    D_8014930C = value;
  }

  next = (u8*)D_1F800044;
  limit = *(s32*)(next + 0x38u);
  target = *(s32*)(next + 0x10u);
  if (limit == target) {
    func_8015DF18(513);
    flags = (u8*)D_1F800044;
    flags[9u] = 0u;
    serial = (u8*)D_1F800044;
    *(volatile s32*)(flags + 0x3Cu) = 0;
    *(volatile s32*)(flags + 0x14u) = 0;
    *(volatile s32*)(flags + 0x20u) = 0;
    serial[0x0Au] = 0u;
    if (D_80149332 == 0u) {
      serial = (u8*)D_1F800044;
      D_8014932E = 0;
      *(volatile s32*)(serial + 0x10u) = 0;
      serial[1u] = (u8)(serial[1u] + 1u);
    }
    func_8014D6B8(4);
  } else {
    if (limit == target + D_801E1D58[next[6u]].first) {
      *(volatile s32*)(next + 0x20u) = D_801E1D58[next[6u]].second;
    }

    state = (u8*)D_1F800044;
    delta = *(s32*)(state + 0x14u) + *(s32*)(state + 0x20u);
    *(volatile s32*)(state + 0x14u) = delta;
    if ((*(s32*)(state + 0x10u) + D_801E1D58[state[6u]].first) <
            *(s32*)(state + 0x38u) &&
        delta < 0) {
      *(volatile s32*)(state + 0x14u) = 0;
      *(volatile s32*)(state + 0x20u) = 0;
    }

    step = (u8*)D_1F800044;
    *(volatile s32*)(step + 0x34u) = *(s32*)(step + 0x34u) + *(s32*)(step + 0x0Cu);
    *(volatile s32*)(step + 0x38u) = *(s32*)(step + 0x38u) - 0x4000;
    *(volatile s32*)(step + 0x3Cu) = *(s32*)(step + 0x3Cu) + *(s32*)(step + 0x14u);
    if (*(s32*)(step + 0x38u) == 0x3C0000) {
      D_8014932E = 0x40;
    }
    func_8014D978();
  }
  func_8014D290();
}
