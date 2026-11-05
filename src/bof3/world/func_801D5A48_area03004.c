#include "bof3/world/area03004_internal.h"

/* @source 0x801D5A48
 * @behavior AREA030 panel-step initializer (handler index 2 of the work-byte-3
 * table at 0x801E1FD4): requests resource 0x21 through the shared func_8014DD3C
 * helper, writes 0x80 to work-record byte 0x24, writes 1 to bytes 0x29 and 0x2A,
 * clears byte 0x48, stores the halfwords 0xA2 at 0x2E and 0xC2 at 0x30, clears
 * bytes 0x5D..0x5F, 9, 0xA and 6 twice, clears the word at 0xC, stores 4 at the
 * word 0x18, requests resource 0 through the shared func_8014D6B8 helper and
 * finally increments dispatch byte 3 of the record published at the scratchpad
 * cursor 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D5A48(void) {
  u8* temp_v1;
  u8* temp_a0;
  u8* temp_v1_2;

  func_8014DD3C(0x21);
  D_1F800044[0x24] = 0x80;
  D_1F800044[0x29] = 1;
  D_1F800044[0x2a] = 1;
  temp_v1 = D_1F800044;
  temp_v1[0x48] = 0;
  temp_a0 = D_1F800044;
  *(u16*)(temp_v1 + 0x2e) = 0xa2;
  *(u16*)(temp_v1 + 0x30) = 0xc2;
  temp_a0[0x5d] = 0;
  D_1F800044[0x5e] = 0;
  D_1F800044[0x5f] = 0;
  D_1F800044[9] = 0;
  D_1F800044[10] = 0;
  D_1F800044[6] = 0;
  temp_v1_2 = D_1F800044;
  *(u32*)(temp_v1_2 + 0xc) = 0;
  *(u32*)(temp_v1_2 + 0x18) = 4;
  temp_v1_2[6] = 0;
  func_8014D6B8(0);
  {
    u8* temp_v1_3 = D_1F800044;
    u8 state = temp_v1_3[3];
    temp_v1_3[3] = state + 1;
  }
}
