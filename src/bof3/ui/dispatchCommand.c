#include "bof3/ui/shop00_internal.h"

/* @source 0x801E1B80
 * @behavior jump-table dispatcher: calls commandHandlerTable[arg0], passing the
 *           zero-extended command id scaled by 4 as the handler argument.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchCommand(u8 arg0) {
  u32 index = (u32)arg0 << 2;

  commandHandlerTable[arg0](index);
}
