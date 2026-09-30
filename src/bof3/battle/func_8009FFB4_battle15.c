#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FFB4
 * @behavior Applies the 0xBFC flags value to the active battler index byte
 *   D_80146394 through func_800A36F0, then negates the selection halfword
 *   selected by that same index through func_8009DE8C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009FFB4(void) {
  func_800A36F0(D_80146394, 0xBFC);
  func_8009DE8C();
}
