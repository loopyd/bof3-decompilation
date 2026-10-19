#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches through the fixed three-entry panel-task forwarding table,
 * preserving the caller's argument block for the selected callee.
 * @source 0x801EA1E0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801EA1E0(u8* arg0, s16* arg1, s16* arg2, s16* arg3, s16* arg4,
                  u8* arg5, s16* arg6, u8* selector) {
  Battle03ForwardingTable table;

  table = panelForwardingHandlers0fec;
  table.handlers[*selector](arg0, arg1, arg2, arg3, arg4, arg5, arg6, selector);
}
