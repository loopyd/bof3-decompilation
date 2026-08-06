#include "internal.h"

extern volatile u8 D_8014648A;
extern volatile u8 D_8014648B;
extern volatile u8 D_80146494;

/* @behavior selects EMI loader mode 6 and phase 2, then clears its busy flag.
 * @source 0x80162CD8
 */
void selectEmiLoaderMode6(void) {
  D_8014648A = 6;
  D_80146494 = 0;
  D_8014648B = 2;
}
