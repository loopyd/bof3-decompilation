#include "internal.h"

/* @behavior dispatches one of two world-front updates from the current
 * sub-state, then ticks the shared waiting path.
 * @source 0x80198170 func_80198170
 */
void func_80198170(void) {
  if (D_80143B92 == 0) {
    func_80198f1c();
  } else {
    func_801981d4();
  }
  func_801992b8();
}
