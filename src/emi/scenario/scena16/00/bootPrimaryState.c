#include "internal.h"

extern u16 D_801448FC;
extern u8  D_801448FF;
extern u32 D_80144900;
extern u32 D_80144904;
extern u16 D_80146258;
extern u32 D_80146864;
extern s8  D_80146872;
extern u8  D_8014832E;

/* @behavior boots the primary SCENA16 state and waits for slot 6 to finish.
 * @source 0x801F6CCC
 */
void bootPrimaryState(void) {
  func_801C1DF0(0u);
  D_8014832E = 0x1fu;
  func_8019FA28(4u, 0x1a0000u, 0x88000u, 5u);
  D_801448FC = 4u;
  D_80144900 = 0x1a0000u;
  D_80146258 |= 0x240u;
  D_80144904 = 0x88000u;
  D_801448FF = 5u;
  func_80161BBC(6u);

  while (!func_80162D00()) {
    func_8014B87C(1u);
  }

  D_80146864 = 0u;
  D_80146872 = 1;
}
