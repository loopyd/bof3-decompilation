#include "internal.h"

/* @behavior seeds one routed setup path and enters secondary state 2 on success.
 * @source 0x801f6eb0 FUN_801f6eb0
 */
void func_801f6eb0(void) {
  if (func_8015b5d4(SCENA16_D_8014686C, 0) == 0) {
    SCENA16_D_8014832E = 0u;
    func_8015b580(SCENA16_D_8014686C, 0);
    func_8015c088();
    SCENA16_D_80146874 = 2;
  }

  if (SCENA16_D_80143F03 == 2u) {
    SCENA16_D_8014832E = 0x1fu;
    func_8015c100();
  }
}
