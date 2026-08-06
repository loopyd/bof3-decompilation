#include "internal.h"

/* @behavior dispatches one of two world-front updates from the current
 * sub-state, then ticks the shared waiting path.
 * @source 0x80198170
 */
void dispatchWorldUpdate(void) {
  if (D_80143B92 == 0) {
    func_80198F1C();
  } else {
    updateWorldPosition();
  }
  func_801992B8();
}
