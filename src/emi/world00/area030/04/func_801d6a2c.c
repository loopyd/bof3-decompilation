#include "internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @behavior seeds the AREA030 scratch record for the later menu phase and bumps
 * scratch state byte `0x03`.
 * @source 0x801d6a2c FUN_801d6a2c
 */
void func_801d6a2c(void) {
  volatile u8* temp_a0;
  volatile u8* temp_v0;
  volatile u8* temp_v1;
  volatile u8* temp_v1_2;

  func_8014dd3c(0x47);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x24] = 0x80;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x29] = 1;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x2a] = 0;
  temp_v1 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_a0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v1[0x48] = 0;
  *(volatile u16*)(temp_v1 + 0x2e) = 0xf0;
  *(volatile u16*)(temp_v1 + 0x30) = 0xba;
  temp_a0[9] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[10] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[6] = 0;
  temp_v0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_a0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v0[0x5d] = 0;
  *(volatile u32*)(temp_v0 + 0xc) = 0x6a;
  *(volatile u32*)(temp_v0 + 0x10) = 0x6a;
  temp_a0[0x5e] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5f] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x4b] = 0xff;
  temp_v1_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v1_2[3] = temp_v1_2[3] + 1;
}
