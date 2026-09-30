#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1B30
 * @behavior While the shared frontend mode byte at 0x80143BB0 is clear, starts
 * action 0x97 through func_80150224, marks that mode byte as 2, and steps the
 * fairy progress byte back by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1B30(void) {
  u8* mode = &D_80143BB0;

  if (*mode == 0) {
    func_80150224(0x97);
    fairyProgress[0] -= 1;
    *mode = 2;
  }
}
