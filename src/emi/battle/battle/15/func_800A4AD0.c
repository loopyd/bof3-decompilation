#include "internal.h"

/* @initializes several global state variables.
 * @source 0x800A4AD0
 */
void func_800A4AD0(void) {
  *D_801462E4 = 0;
  D_801462E6 = 0;
  *D_801462E3 = D_801462E5 + 3;
  D_801462E5 |= 0x80;
}
