#include "internal.h"

/* @calls func_800A403C with argument 1
 * @source 0x8009E7C4
 * @behavior forwards selector 1 to func_800A403C
 */
void forwardSelector1(void) {
  func_800A403C(1);
}
