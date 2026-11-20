#include "bof3/world/area03004_internal.h"

/* @behavior Publish a copy of work word 0x10 clamped to 0x150000..0x3C0000 at
 * D_8014930C. While words 0x38 and 0x10 differ, update the original motion
 * accumulators, position and target, then step the pair sequence. On equality
 * request cue 513, clear the countdown and accumulators, and conditionally
 * clear the scroll and advance handler byte 1. The equality path calls
 * func_8014D6B8(4); every path calls func_8014D290.
 * @source 0x801D23F4
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishClampedScrollAndIntegrateMotionOrAdvanceHandlerWorld00Area03004(void) {
  u8* cursor;
  u8* next;
  u8* state;
  u8* step;
  u8* flags;
  u8* serial;
  s32 scrollValue;
  s32 scrollWord;
  s32 targetWord;
  s32 integratedStep;

  cursor = (u8*)D_1F800044;
  scrollValue = *(s32*)(cursor + 0x10u);
  if (scrollValue > 0x3BFFFF) {
    D_8014930C = 0x3C0000;
  } else if (scrollValue <= 0x14FFFF) {
    D_8014930C = 0x150000;
  } else {
    D_8014930C = scrollValue;
  }

  next = (u8*)D_1F800044;
  targetWord = *(s32*)(next + 0x38u);
  scrollWord = *(s32*)(next + 0x10u);
  if (targetWord == scrollWord) {
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
    if (targetWord == scrollWord + D_801E1D58[next[6u]].first) {
      *(volatile s32*)(next + 0x20u) = D_801E1D58[next[6u]].second;
    }

    state = (u8*)D_1F800044;
    integratedStep = *(s32*)(state + 0x14u) + *(s32*)(state + 0x20u);
    *(volatile s32*)(state + 0x14u) = integratedStep;
    if ((*(s32*)(state + 0x10u) + D_801E1D58[state[6u]].first) <
            *(s32*)(state + 0x38u) &&
        integratedStep < 0) {
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
