#include "internal.h"

/* @behavior Initializes the game runtime, then runs the permanent frame loop,
 * including the guarded EMI service/reset path.
 * @source 0x8014AAC8
 */
void func_8014AAC8(void) {
  u8* work;

  func_8014AA04();
  D_8018B300 = 0;
  func_8014ACA0();
  func_8014AEE0();
  DrawSyncCallback(&D_8014B17C);
  func_8014B854(0, func_8014EA80);

  for (;;) {
    VSync(2);
    rand();
    PutDispEnv((DISPENV*)D_80143E68);
    PutDrawEnv((DRAWENV*)(D_80143E68 + 0x14));
    func_8014E22C();
    func_8014E6D0();
    DrawOTag((u_long*)(D_80143E68 + 0x8c));
    func_8014AFC0();
    func_8015D044();

    D_80143D44 ^= 1;
    work = D_80143D48 + D_80143D44 * 0x90;
    D_80143E68 = work;
    ClearOTagR((u_long*)(work + 0x70), 8);
    func_8014B020();

    if ((D_80145AA4 & 0x900) == 0x900) {
      if (D_80143F44 == 60) {
        if (emi_loader_is_ready() != 0) {
          D_80143F44 = 0;
          func_8015CEBC();
          func_8014B33C();
          func_8014B854(0, func_8014EA80);
        }
      } else {
        D_80143F44++;
      }
    } else {
      D_80143F44 = 0;
    }

    func_8014B73C();
    func_80163010();
    D_80143EF8 = VSync(1);
    DrawSync(0);
    func_8014B0F0();
    D_80143E6C++;
  }
}
