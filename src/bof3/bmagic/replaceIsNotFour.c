#include "bof3/bof3.h"

/* Shared battle state byte outside this overlay. The battle entry routines
 * store 4 into it when a battle starts. */
extern u8 D_80146375; /* @source 0x80146375 @kind unknown */

/* Shared battle kind halfword outside this overlay. The sibling kind
 * resolvers at src/bof3/bmagic/resolveKindResultCodeBmagicMagic00800_801F0230.c
 * and src/bof3/bmagic/resolveKindResultCodeWhenBattleStateIsFourBmagicMagic08803_801EF34C.c
 * read the same halfword as the selected battle kind. */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* @source 0x801EF0CC
 * @behavior Reads the shared battle state byte at 0x80146375 once. While it is
 *           not four it reads the battle kind halfword at 0x801463C0 and, when
 *           that kind is 0x136, overwrites it with the halfword 0x53 with a sh.
 *           While the state byte is four it reads no kind. Both paths converge
 *           on the one return that materializes zero in $v0, and the body is a
 *           frameless leaf of 0x3C bytes. The caller at 0x801EED68 stores that
 *           returned byte in work byte 4 of the scratchpad work object, the
 *           slot the sibling kind resolvers feed, and passes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 replaceIsNotFour(
    void) {
  if (D_80146375 != 4) {
    if (D_801463C0 == 0x136) {
      D_801463C0 = 0x53;
    }
  }
  return 0;
}
