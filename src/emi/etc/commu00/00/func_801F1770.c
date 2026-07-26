#include "internal.h"

/* @source 0x801F1770
 * @behavior selects message 0x259 and advances its local state byte
 */
void func_801F1770(void) {
  func_80161FDC(0x259u);
  D_801448EC[0] += 1;
}
