#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E01C
 * @behavior Applies the 0x100 flags value to the active battler index byte
 *   D_80146394 through func_800A36F0, then runs the selection-input pass
 *   func_800A3F28.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009E01C(void) {
  func_800A36F0(D_80146394, 0x100);
  func_800A3F28();
}
