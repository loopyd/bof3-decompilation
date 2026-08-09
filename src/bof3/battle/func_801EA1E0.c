#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches through the fixed three-entry panel-task forwarding table,
 * preserving the caller's argument block for the selected callee.
 * @source 0x801EA1E0
 * @status partial
 * @match 40.00
 * @residual non-exact live audit: 12/29 instructions; 116 original bytes versus 120 current.
 */
void func_801EA1E0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, u8* selector) {
  Battle03ForwardingHandler local_18[3];
  register u32 const*       table;

  table = (u32 const*)0x801d0fecu;
  local_18[0] = (Battle03ForwardingHandler)table[0];
  local_18[1] = (Battle03ForwardingHandler)table[1];
  local_18[2] = (Battle03ForwardingHandler)table[2];
  local_18[*selector](arg0, arg1, arg2, arg3, arg4, arg5, arg6, selector);
}
