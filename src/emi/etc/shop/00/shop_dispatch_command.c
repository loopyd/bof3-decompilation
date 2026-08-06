#include "internal.h"

/* @source 0x801E1B80
 * @behavior jump-table dispatcher: calls shop_command_handlerTable[arg0], passing the
 *           zero-extended command id scaled by 4 as the handler argument.
 */
void shop_dispatch_command(u8 arg0) {
  u32 index = (u32)arg0 << 2;

  shop_command_handlerTable[arg0](index);
}
