#include "internal.h"

extern vu16 D_801448FC;
extern vu8  D_801448FF;
extern vu32 D_80144900;
extern vu32 D_80144904;
extern vu16 D_80146258;
extern vu32 D_80146864;
extern vs8  D_80146872;
extern vu8  D_8014832E;

/* @behavior boots the primary SCENA16 state and waits for slot 6 to finish.
 * @source 0x801f6ccc FUN_801f6ccc
 */
void func_801f6ccc(void) {
  func_801c1df0(0u);
  D_8014832E = 0x1fu;
  func_8019fa28(4u, 0x1a0000u, 0x88000u, 5u);
  D_801448FC = 4u;
  D_80144900 = 0x1a0000u;
  D_80146258 |= 0x240u;
  D_80144904 = 0x88000u;
  D_801448FF = 5u;
  func_80161bbc(6u);

  while (!func_80162d00()) {
    func_8014b87c(1u);
  }

  D_80146864 = 0u;
  D_80146872 = 1;
}
