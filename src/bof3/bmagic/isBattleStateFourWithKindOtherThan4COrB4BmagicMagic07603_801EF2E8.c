#include "bof3/bof3.h"

extern u8 D_80146375; /* @source 0x80146375 @kind unknown */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* @source 0x801EF2E8
 * @behavior Returns zero unless battle state byte 0x80146375 is four.
 *           In that state returns zero for kinds 0x4C and 0xB4 and one
 *           for every other kind. The caller stores the result in work byte 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 isBattleStateFourWithKindOtherThan4COrB4BmagicMagic07603_801EF2E8(void) {
  if (D_80146375 == 4) {
    switch (D_801463C0) {
      case 0x4C:
      case 0xB4:
        return 0;
      case 0x4D:
        return 1;
      default:
        return 1;
    }
  }
  return 0;
}
