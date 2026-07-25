#include "internal.h"

/* @behavior clears D_80143F4A and bit 5 of D_80143FBC unless scenario ID is 2.
 * @source 0x801C5C7C
 */
void func_801C5C7C(void) {
  u8* flags;

  if (D_80143BB0 != 2) {
    flags = &D_80143FBC;
    D_80143F4A = 0;
    *flags &= 0xDF;
  }
}
