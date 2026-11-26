#include "bof3/ui/game00_internal.h"

/* @source 0x80197AA4
 * @behavior waits for the selected frontend route, configures its scenario
 * coordinates and stream mode, then advances the New Game/load preparation
 * chain.
 * @status exact
 * @match 100.00
 * @residual none
 * Note: the four multi-access objects (D_80146325, D_8014930A, D_8014930E,
 * D_80143B92) are reached through address-materialising locals because the
 * original materialises their addresses with lui/addiu and then uses
 * zero-displacement load/store pairs, which plain extern scalar access does not
 * emit. */
void stateFrontendRoutePrep(void) {
  u8 ready;
  u8 *flags;
  u16 *coord_x;
  u16 *coord_y;
  u16 *entry_serial;
  u16 serial_value;
  u32 step_y;
  u32 value;
  u32 base;

  ready = stateRoutesInitializeTint();
  func_801A06D8();
  runFrameFinalizationServices();
  if (ready) {
    func_80161808(1);
    flags = &D_80146325;
    if ((*flags & 1) == 0) {
      do {
        coord_x = &D_8014930A;
        *coord_x += D_801C7B74[D_801462EC * 2];
      } while (0);
      step_y = (u8)D_801C7B75[D_801462EC * 2];
      do {
        value = D_801CD954;
        base = D_80149318;
        D_8014932E = 64;
        coord_y = &D_8014930E;
        value -= base;
        value = (s32)value >> 2;
        value = (s32)value >> 2;
        step_y <<= 12;
        step_y <<= 12;
        D_80149330 = value;
      } while (0);
      {
        u16 current_y;

        current_y = *coord_y;
        *coord_y = current_y + ((s32)step_y >> 24);
      }
    }
    if ((*flags & 0x10) == 0) {
      if ((D_801462EC & 2) != 0) {
        func_8016728C(D_80145024 & 0x7f, 2);
      } else {
        func_8016728C(D_80145024 & 0x7f, 1);
      }
    }
    do {
      entry_serial = &D_80143B92;
      serial_value = *entry_serial;
    } while (0);
    D_8014933E = 4;
    D_80149333 = 2;
    {
      u8 route;

      do {
        route = D_801462EC;
        serial_value += 1;
        D_8014933F = route & 1;
      } while (0);
      *entry_serial = serial_value;
    }
  }
}
