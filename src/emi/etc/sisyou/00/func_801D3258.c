#include "internal.h"

/* @source 0x801D3258
 * @behavior starts the selected entry's +4 action unless the local state is already two.
 */
void func_801D3258(void) {
  u8* state = &D_80143BB0;

  if (*state == 2) {
    return;
  }
  func_80150224((s16)(D_801D41BC[D_801448ED] + 4));
  *state = 2;
  D_801D4285 = 6;
}
