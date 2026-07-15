#include "internal.h"

/* @behavior waits for the shared frontend transition to finish, ticking the
 * GAME.EMI#0 update path unless an idle transition may be retriggered.
 * @source 0x80198bc4 func_80198bc4
 */
void func_80198bc4(u32 arg0) {
  u8 force_update;

  force_update = arg0;
  while (1) {
    func_8014b87c(1);
    if (D_80143C40 == 0 && force_update == 0) {
      func_8014b87c(1);
      break;
    }
    func_801991b8();
    if (D_80143C40 == 0) {
      break;
    }
  }
}
