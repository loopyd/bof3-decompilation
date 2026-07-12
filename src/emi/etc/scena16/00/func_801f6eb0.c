#include "internal.h"

/* @behavior seeds one routed setup path and enters secondary state 2 on success.
 * @source 0x801f6eb0 FUN_801f6eb0
 */
void func_801f6eb0(void) {
  if (func_8015b5d4(BOF3_SCENA16_DAT_8014686c, 0) == 0) {
    BOF3_SCENA16_DAT_8014832e = 0u;
    func_8015b580(BOF3_SCENA16_DAT_8014686c, 0);
    func_8015c088();
    BOF3_SCENA16_DAT_80146874 = 2;
  }

  if (BOF3_SCENA16_DAT_80143f03 == 2u) {
    BOF3_SCENA16_DAT_8014832e = 0x1fu;
    func_8015c100();
  }
}
