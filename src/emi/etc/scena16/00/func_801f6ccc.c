#include "internal.h"

extern vu16 DAT_801448fc;
extern vu8  DAT_801448ff;
extern vu32 DAT_80144900;
extern vu32 DAT_80144904;
extern vu16 DAT_80146258;
extern vu32 DAT_80146864;
extern vs8  DAT_80146872;
extern vu8  DAT_8014832e;

/* @behavior boots the primary SCENA16 state and waits for slot 6 to finish.
 * @source 0x801f6ccc FUN_801f6ccc
 */
void func_801f6ccc(void) {
  func_801c1df0(0u);
  DAT_8014832e = 0x1fu;
  func_8019fa28(4u, 0x1a0000u, 0x88000u, 5u);
  DAT_801448fc = 4u;
  DAT_80144900 = 0x1a0000u;
  DAT_80146258 |= 0x240u;
  DAT_80144904 = 0x88000u;
  DAT_801448ff = 5u;
  func_80161bbc(6u);

  while (!func_80162d00()) {
    func_8014b87c(1u);
  }

  DAT_80146864 = 0u;
  DAT_80146872 = 1;
}
