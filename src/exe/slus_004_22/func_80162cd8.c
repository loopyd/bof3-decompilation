#include "internal.h"

extern vu8 D_8014648A;
extern vu8 D_8014648B;
extern vu8 D_80146494;

/* @behavior selects EMI loader mode 6 and phase 2, then clears its busy flag.
 * @source 0x80162CD8
 */
void func_80162CD8(void) {
  D_8014648A = 6;
  D_80146494 = 0;
  D_8014648B = 2;
}
