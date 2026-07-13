#include "internal.h"

/* @behavior advances the second frontend callback bank after its resource,
 * update, and local gate are all ready.
 * @source 0x80197c1c func_80197c1c
 */
void func_80197c1c(void) {
  u8   local_ready;
  u16* state;

  local_ready = func_801bf11c();
  func_801a06d8();
  func_801992b8();
  if (emi_loader_is_ready() && local_ready && DAT_80149332 == 0) {
    state = &DAT_80143b92;
    (*state)++;
  }
}
