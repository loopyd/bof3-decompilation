#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @source 0x801D6414
 * @behavior AREA030 scratch work-record initializer for the 0x2B step: requests
 * resource 0x2B through the shared func_8014DD3C helper, stores 0x80 in
 * work-record byte 0x24, clears bytes 0x29/0x2A, stores 2 in byte 0x48, clears
 * bytes 0x5D..0x5F, stores 0x8000 in the step words at 0x40/0x44, stores 0x40
 * in byte 9, clears byte 6, requests resource 3 through the shared func_8014D6B8
 * helper and finally increments the dispatch byte 3 of the record published at
 * the scratchpad cursor 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D6414(void) {
  volatile u8* temp_a0;
  volatile u8* temp_a0_2;
  volatile u8* temp_v0;
  volatile u8* temp_v1;
  volatile u8* temp_v1_2;

  func_8014DD3C(0x2b);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x24] = 0x80;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x29] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x2a] = 0;
  temp_v1 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v1[0x48] = 2;
  temp_v0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v0[0x5d] = 0;
  temp_a0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u32*)(temp_v0 + 0x40) = 0x8000;
  *(volatile u32*)(temp_v0 + 0x44) = 0x8000;
  temp_a0[0x5e] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5f] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[9] = 0x40;
  temp_v1_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(u8*)(temp_v1_2 + 6) = 0;
  func_8014D6B8(3);
  temp_a0_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 state = temp_a0_2[3];
    *(u8*)(temp_a0_2 + 3) = state + 1;
  }
}
