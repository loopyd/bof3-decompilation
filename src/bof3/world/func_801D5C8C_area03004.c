#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @behavior initializes the AREA030 scratch work record for the next panel
 * step: calls the shared func_8014DD3C helper with 0x47, writes 0x80 to scratch
 * offset 0x24, writes 1 to offset 0x29, clears offsets 0x2A and 0x48, stores the
 * scratch halfwords 0xEC at 0x2E and 0xA2 at 0x30, clears scratch bytes
 * 0x5D..0x5F and 0x09/0x0A/0x06, clears the scratch word at 0x0C, stores 4 at
 * the scratch word 0x18, then increments the scratch state byte at offset 0x03.
 * @source 0x801D5C8C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D5C8C(void) {
  volatile u8* temp_v1;
  volatile u8* temp_a0;
  volatile u8* temp_v1_2;
  volatile u8* temp_a0_2;

  func_8014DD3C(0x47);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x24] = 0x80;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x29] = 1;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x2a] = 0;
  temp_v1 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v1[0x48] = 0;
  temp_a0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u16*)(temp_v1 + 0x2e) = 0xec;
  *(volatile u16*)(temp_v1 + 0x30) = 0xa2;
  temp_a0[0x5d] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5e] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5f] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[9] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[10] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[6] = 0;
  temp_v1_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v1_2[6] = 0;
  temp_a0_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u32*)(temp_v1_2 + 0xc) = 0;
  *(volatile u32*)(temp_v1_2 + 0x18) = 4;
  {
    u8 state = temp_a0_2[3];
    *(u8*)(temp_a0_2 + 3) = state + 1;
  }
}
