#include "internal.h"

/* @behavior waits for the active frontend operation and EMI stream, clears
 * the next-operation state, records its reset result, and advances.
 * @source 0x80197e54 func_80197e54
 */
void func_80197e54(void) {
  u8   local_ready;
  u16* state;

  local_ready = func_801bf78c();
  func_801a06d8();
  func_801992b8();
  if (emi_loader_is_ready() && local_ready) {
    D_8014932E = 0;
    D_80146329 = 0;
    D_801462E0 = 0;
    D_801462E1 = 0;
    D_801462E2 = 0;
    D_801462E3 = 0;
    D_801462E4 = 0;
    D_801462F0 = func_801bdb7c(1);
    state = &D_80143B92;
    (*state)++;
  }
}
