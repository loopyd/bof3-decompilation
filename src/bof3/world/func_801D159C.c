#include "bof3/world/area03004_internal.h"

/* @behavior draws the AREA030 status-panel header row at the caller origin: selects
 * draw mode 5/3 and stamps sprite 0x0D at (x + 24, y + 8), then mode 0/3 for sprites
 * 0x0E at (x, y) and 0x0F at (x + 256, y). It then prints the shared counter 62 minus
 * 0x80144002 into the shared 0x80145AD4 buffer through the "%2d" format while the
 * shared byte 0x80143FC9 is set, clamping a negative result to zero, and draws the
 * (x + 240, y + 26) row with mode 2 only when that counter reached 50, the shared
 * phase byte reads 4 and bit 2 of the shared frame counter is set, and with mode 0
 * otherwise. Finally it forwards both caller coordinates to func_801D195C.
 * @source 0x801D159C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D159C(s16 arg0, s16 arg1) {
  u8* prim;
  s32 shade;

  submitTpageDrawMode(5, 3);
  func_801E0DCC(0x0D, 3, (s16)(arg0 + 24), (s16)(arg1 + 8));
  submitTpageDrawMode(0, 3);
  func_801E0DCC(0x0E, 3, arg0, arg1);
  func_801E0DCC(0x0F, 3, (s16)(arg0 + 256), arg1);

  if (D_80143FC9 == 0u) {
    shade = 0;
  } else {
    shade = 62 - D_80144002;
  }
  if ((s8)shade < 0) {
    shade = 0;
  }
  prim = D_80145AD4;
  func_8017E3F4((char*)prim, (char*)D_801D0C04, (s8)shade);

  if (((s8)shade >= 50) && (D_80145E92 == 4u) && ((D_80143E6C & 4) != 0)) {
    func_8014FF0C((s16)(arg0 + 240), (s16)(arg1 + 26), 2, D_80145AD4);
  } else {
    func_8014FF0C((s16)(arg0 + 240), (s16)(arg1 + 26), 0, D_80145AD4);
  }
  func_801D195C(arg0, arg1);
}
