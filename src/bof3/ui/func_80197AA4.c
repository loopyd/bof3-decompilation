#include "bof3/ui/game00_internal.h"

/* @source 0x80197AA4
 * @behavior waits for the selected frontend route, configures its scenario
 * coordinates and stream mode, then advances the New Game/load preparation
 * chain.
 * @status partial
 * @match 56.38
 * @residual non-exact live audit: 53/94 instructions, 376 original bytes versus
 * 372 current. The residual is instruction scheduling, not shape: the original
 * completes the 0x8014930A read-modify-write before loading the second route
 * step byte and defers the 0x8014930E update until after the 0x80149330 store,
 * while this build sinks both read-modify-write pairs past the 0x80149330
 * accumulator. The same scheduling difference fills the flag-test delay slot
 * with the 0x40 constant instead of the original nop, which leaves this build
 * one instruction short and shifts every following branch displacement.
 * The flag pointer also lands in $t0 instead of the original caller-saved $a2.
 * Statement order and declaration order do not change the schedule, so the
 * remaining gap is compiler allocation/scheduling only.
 * Note: the four multi-access objects (D_80146325, D_8014930A, D_8014930E,
 * D_80143B92) are reached through address-materialising locals because the
 * original materialises their addresses with lui/addiu and then uses
 * zero-displacement load/store pairs, which plain extern scalar access does not
 * emit. */
void func_80197AA4(void) {
  u8 ready;
  u8 *flags;
  u16 *coord_x;
  u16 *coord_y;
  u16 *entry_serial;

  ready = func_801BEE5C();
  func_801A06D8();
  runFrameFinalizationServices();
  if (ready) {
    func_80161808(1);
    flags = &D_80146325;
    if ((*flags & 1) == 0) {
      coord_x = &D_8014930A;
      coord_y = &D_8014930E;
      *coord_x += D_801C7B74[D_801462EC * 2];
      *coord_y += D_801C7B75[D_801462EC * 2];
      D_8014932E = 64;
      D_80149330 = (s32)(D_801CD954 - D_80149318) >> 4;
    }
    if ((*flags & 0x10) == 0) {
      if ((D_801462EC & 2) != 0) {
        func_8016728C(D_80145024 & 0x7f, 2);
      } else {
        func_8016728C(D_80145024 & 0x7f, 1);
      }
    }
    D_8014933E = 4;
    D_80149333 = 2;
    D_8014933F = D_801462EC & 1;
    entry_serial = &D_80143B92;
    (*entry_serial)++;
  }
}
