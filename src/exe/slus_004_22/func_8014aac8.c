#include "internal.h"

/* @behavior Initializes the game runtime, then runs the permanent frame loop,
 * including the guarded EMI service/reset path.
 * @source 0x8014aac8 func_8014aac8
 */
void func_8014aac8(void) {
  u8* work;

  func_8014aa04();
  DAT_8018b300 = 0;
  func_8014aca0();
  func_8014aee0();
  func_8017b2d4(&DAT_8014b17c);
  func_8014b854(0, func_8014ea80);

  for (;;) {
    VSync(2);
    func_8017e3d4();
    PutDispEnv((DISPENV*)DAT_80143e68);
    PutDrawEnv((DRAWENV*)(DAT_80143e68 + 0x14));
    func_8014e22c();
    func_8014e6d0();
    DrawOTag((u_long*)(DAT_80143e68 + 0x8c));
    func_8014afc0();
    func_8015d044();

    DAT_80143d44 ^= 1;
    work = DAT_80143d48 + DAT_80143d44 * 0x90;
    DAT_80143e68 = work;
    ClearOTagR((u_long*)(work + 0x70), 8);
    func_8014b020();

    if ((DAT_80145aa4 & 0x900) == 0x900) {
      if (DAT_80143f44 == 60) {
        if (emi_loader_is_ready() != 0) {
          DAT_80143f44 = 0;
          func_8015cebc();
          func_8014b33c();
          func_8014b854(0, func_8014ea80);
        }
      } else {
        DAT_80143f44++;
      }
    } else {
      DAT_80143f44 = 0;
    }

    func_8014b73c();
    func_80163010();
    DAT_80143ef8 = VSync(1);
    DrawSync(0);
    func_8014b0f0();
    DAT_80143e6c++;
  }
}
