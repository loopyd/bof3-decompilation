#include "internal.h"

extern vu8 DAT_8014648a;
extern vu8 DAT_8014648b;
extern vu8 DAT_80146494;

/* @behavior selects EMI loader mode 6 and phase 2, then clears its busy flag.
 * @source 0x80162cd8 func_80162cd8
 */
void func_80162cd8(void) {
  DAT_8014648a = 6;
  DAT_80146494 = 0;
  DAT_8014648b = 2;
}
