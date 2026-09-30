#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @source 0x801D5B70
 * @behavior AREA030 panel step-back initializer: requests resource 0x47 through
 * the shared func_8014DD3C helper, stores 0x80 in work-record byte 0x24, 1 in
 * byte 0x29, clears bytes 0x2A/0x48, stores the halfwords 0xE8 at 0x2E and 0xC2
 * at 0x30, clears bytes 9/0xA/0x5D/0x5E/0x5F, requests resource 0xA through the
 * shared func_8014D6B8 helper and finally decrements the dispatch byte 3 of the
 * record published at the scratchpad cursor 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D5B70(void) {
  volatile u8* temp_v1;
  volatile u8* temp_a0;
  volatile u8* temp_v0;
  volatile u8* temp_v1_2;

  func_8014DD3C(0x47);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x24] = 0x80;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x29] = 1;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x2a] = 0;
  temp_v1 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  temp_v1[0x48] = 0;
  temp_a0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u16*)(temp_v1 + 0x2e) = 0xe8;
  *(volatile u16*)(temp_v1 + 0x30) = 0xc2;
  temp_a0[9] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[10] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5d] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5e] = 0;
  temp_v0 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(u8*)(temp_v0 + 0x5f) = 0;
  func_8014D6B8(0xa);
  temp_v1_2 = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 state = temp_v1_2[3];
    *(u8*)(temp_v1_2 + 3) = state - 1;
  }
}
