#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4BB4
 * @behavior Once the EXE-side EMI loader reports ready through func_80162D00,
 * requests engine action 0x1B through func_8014E3A0, raises the shared flag
 * byte D_80145988, and arms the battle selection state bytes:
 * D_801462E2 = 1, D_801462E3 = 1, D_801462E4 = 0. Nothing is stored while the
 * loader is not ready.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A4BB4(void) {
  if (func_80162D00() != 0) {
    func_8014E3A0(0x1B);
    D_80145988 = 1;
    D_801462E2 = 1;
    D_801462E3 = 1;
    D_801462E4 = 0;
  }
}
