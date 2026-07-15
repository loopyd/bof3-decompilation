#include "internal.h"

/* @behavior Initializes the game runtime, then runs the permanent frame loop,
 * including the guarded EMI service/reset path.
 * @source 0x8014aac8 func_8014aac8
 */
void func_8014aac8(void) {
  u8* work;

  func_8014aa04();
  D_8018B300 = 0;
  func_8014aca0();
  func_8014aee0();
  func_8017b2d4(&D_8014B17C);
  func_8014b854(0, func_8014ea80);

  for (;;) {
    VSync(2);
    func_8017e3d4();
    PutDispEnv((DISPENV*)D_80143E68);
    PutDrawEnv((DRAWENV*)(D_80143E68 + 0x14));
    func_8014e22c();
    func_8014e6d0();
    DrawOTag((u_long*)(D_80143E68 + 0x8c));
    func_8014afc0();
    func_8015d044();

    D_80143D44 ^= 1;
    work = D_80143D48 + D_80143D44 * 0x90;
    D_80143E68 = work;
    ClearOTagR((u_long*)(work + 0x70), 8);
    func_8014b020();

    if ((D_80145AA4 & 0x900) == 0x900) {
      if (D_80143F44 == 60) {
        if (emi_loader_is_ready() != 0) {
          D_80143F44 = 0;
          func_8015cebc();
          func_8014b33c();
          func_8014b854(0, func_8014ea80);
        }
      } else {
        D_80143F44++;
      }
    } else {
      D_80143F44 = 0;
    }

    func_8014b73c();
    func_80163010();
    D_80143EF8 = VSync(1);
    DrawSync(0);
    func_8014b0f0();
    D_80143E6C++;
  }
}
