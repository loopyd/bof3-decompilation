#include "internal.h"

/* @behavior waits for the active frontend operation and EMI stream, clears
 * the next-operation state, records its reset result, and advances.
 * @source 0x80197E54
 */
void func_80197E54(void) {
  u8   local_ready;
  u16* state;

  local_ready = func_801BF78C();
  func_801A06D8();
  func_801992B8();
  if (emi_loader_is_ready() && local_ready) {
    D_8014932E = 0;
    D_80146329 = 0;
    D_801462E0 = 0;
    D_801462E1 = 0;
    D_801462E2 = 0;
    D_801462E3 = 0;
    D_801462E4 = 0;
    D_801462F0 = func_801BDB7C(1);
    state = &D_80143B92;
    (*state)++;
  }
}
