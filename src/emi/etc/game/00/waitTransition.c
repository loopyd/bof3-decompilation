#include "internal.h"

/* @behavior waits for the shared frontend transition to finish, ticking the
 * GAME.EMI#0 update path unless an idle transition may be retriggered.
 * @source 0x80198BC4
 */
void waitTransition(u32 arg0) {
  u8 force_update;

  force_update = arg0;
  while (1) {
    func_8014B87C(1);
    if (effectBusy == 0 && force_update == 0) {
      func_8014B87C(1);
      break;
    }
    func_801991B8();
    if (effectBusy == 0) {
      break;
    }
  }
}
