#include "bof3/bof3.h"

/* Shared battle state byte outside this overlay. The battle entry routines
 * store 4 into it when a battle starts (src/bof3/battle/func_8009F52C.c), so
 * the guard below keeps the kind mapping off every other battle phase. */
extern u8 D_80146375; /* @source 0x80146375 @kind unknown */

/* Shared battle kind halfword outside this overlay. The sibling battle lifts
 * read it as the selected battle kind
 * (src/bof3/bmagic/resolveKindResultCodeBmagicMagic03903_801EEF3C.c). */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* @source 0x801EF34C
 * @behavior Reads the battle state byte at 0x80146375 once. While it is not
 *           four it reads the battle kind halfword at 0x801463C0 and returns
 *           three, except for the kind 0x112, which it overwrites with 0x5A
 *           before returning zero. While the state byte is four it reads the
 *           kind halfword once and maps it through a sparse switch of eight
 *           cases: kinds 0x1E, 0x58 and 0xBE return one, kinds 0x1F, 0x59 and
 *           0xBF return two, kinds 0x5A and 0xC0 return zero, and every other
 *           kind returns three. The compiler balances that switch into a
 *           comparison tree whose pivots are 0x59, 0x1F, 0xBE, 0x5A, 0xBF and
 *           0xC0, and every leaf jumps to the one return block with its own
 *           constant parked in a branch delay slot, so the body is a leaf with
 *           no frame and 0xE4 bytes. No jal in this payload reached it and no
 *           pointer word names it, so its caller was the overlay loader or a
 *           script-invoked callback, and it takes no arguments.
 * @status partial
 * @match 94.74
 * @residual The last case leaf is arranged differently. For kind 0xC0 the
 *           original branches on bne to the shared return-three block with
 *           0x3 in that branch delay slot and falls through into the inline
 *           return-zero body, while this candidate branches on beq onto the
 *           shared return block with the return-zero move in the delay slot
 *           and reaches return three through a threaded jump. Both forms are
 *           four instructions at the same 0xE4-byte size, so the mismatch is
 *           confined to that leaf: 54 of 57 instructions and 228 of 228 bytes.
 *           Eight other switch and arm spellings were measured and reverted.
 */
u8 resolveKindResultCodeWhenBattleStateIsFourBmagicMagic08803_801EF34C(void) {
  u16 kind;

  if (D_80146375 == 4) {
    kind = D_801463C0;
    switch (kind) {
      case 0x1E:
        return 1;
      case 0x1F:
        return 2;
      case 0x58:
        return 1;
      case 0x59:
        return 2;
      case 0x5A:
        return 0;
      case 0xBE:
        return 1;
      case 0xBF:
        return 2;
      case 0xC0:
        return 0;
    }
  } else if (D_801463C0 == 0x112) {
    D_801463C0 = 0x5A;
    return 0;
  }
  return 3;
}
