#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @behavior seeds the AREA030 scratch record for the later menu phase and bumps
 * scratch state byte `0x03`.
 * @source 0x801D6A2C
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedMenuScratch(void) {
  volatile u8* temp_a0;
  volatile u8* temp_v0;
  volatile u8* temp_v1;
  volatile u8* temp_v1_2;

  WORLD00_AREA030_INIT_PANEL_WORK(
      ((volatile struct scratchpad_state*)0x1f800000u)->scratch,
      temp_v1, temp_a0, 0xf0, 0xba);
  temp_a0[9] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[10] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[6] = 0;
  temp_v0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v0[0x5d] = 0;
  temp_a0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u32*)(temp_v0 + 0xc) = 0x6a;
  *(volatile u32*)(temp_v0 + 0x10) = 0x6a;
  temp_a0[0x5e] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5f] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x4b] = 0xff;
  temp_v1_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 state = temp_v1_2[3];
    *(u8*)(temp_v1_2 + 3) = state + 1;
  }
}
