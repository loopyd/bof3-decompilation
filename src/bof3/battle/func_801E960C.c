#include "bof3/battle/battle03_internal.h"

/* @source 0x801E960C
 * @behavior Dispatches the panel handler selected by state byte +0x03 through the
 * four-entry handler table copied off 0x801D0FD0, then forwards the state field
 * pointers +0xB/+0x1C/+0x14/+0xD/+0x18/+0x8 and the selected enemy work record's
 * +0x94/+0xA0 halfword pointers to func_801EA1E0. When func_801DB524(state byte
 * +0x0A) reports a hit while state byte +0x0D is clear it sets state byte +0x03
 * to 3, and it finally raises bit 0x2000 of that enemy work record's flags
 * halfword +0x82 while its halfword +0x94 is below a quarter of its halfword
 * +0xA0, clearing the bit otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E960C(void) {
  Battle03FourDispatchTable local_20;
  u8*                       record;
  u32                       index;

  local_20 = D_801D0FD0;
  local_20.handlers[D_80148648[3]]();

  index = D_80148648[0xA];
  record = D_801EB35C + index * 0x118;
  func_801EA1E0((s32)(D_80148648 + 0xB), (s32)(record + 0x2C),
                (s32)(D_80148648 + 0x1C), (s32)(D_80148648 + 0x14),
                (s32)(record + 0x20), (s32)(D_80148648 + 0xD),
                (s32)(D_80148648 + 0x18), D_80148648 + 0x8);

  if ((func_801DB524(D_80148648[0xA]) != 0) && (D_80148648[0xD] == 0)) {
    D_80148648[3] = 3;
  }

  if (D_801EB630[D_80148648[0xA] - 3].unk_94 <
      (D_801EB630[D_80148648[0xA] - 3].unk_a0 >> 2)) {
    D_801EB630[D_80148648[0xA] - 3].unk_82 |= 0x2000u;
  } else {
    D_801EB630[D_80148648[0xA] - 3].unk_82 &= 0xdfffu;
  }
}
