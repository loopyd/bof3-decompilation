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
    DAT_8014932e = 0;
    DAT_80146329 = 0;
    DAT_801462e0 = 0;
    DAT_801462e1 = 0;
    DAT_801462e2 = 0;
    DAT_801462e3 = 0;
    DAT_801462e4 = 0;
    DAT_801462f0 = func_801bdb7c(1);
    state = &DAT_80143b92;
    (*state)++;
  }
}
