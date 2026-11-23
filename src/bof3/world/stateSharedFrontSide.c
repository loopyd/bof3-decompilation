#include "bof3/bof3.h"

extern u8 D_80143F03; /* @source 0x80143F03 @kind unknown */

/* Target-local flag transition at 0x801F2C80, an asm row of this target's
 * splat when this lift was written: its own bytes pass bit 2 to the shared
 * bit-clear service 0x8015B5A8 and bit 3 to the shared bit-set service
 * 0x8015B580, both on the shared flag bank 0x80144F28. This declaration binds
 * the call target by that map row. */
void stateSharedSetFlag3(void);

/* @source 0x801F2CC0
 * @behavior Overlay dispatch handler. When the argument word is exactly
 *           0x4D8000 and the shared front-side byte D_80143F03 reads 0, it runs
 *           the target-local flag transition at 0x801F2C80, which clears bit 2
 *           and sets bit 3 of the shared flag bank 0x80144F28. Every other
 *           arrival touches no state, and both paths return 0.
 *
 *           Constant and branch evidence from the original bytes: the argument
 *           word 0x4D8000 is materialised with `lui`/`ori` into $v0 and compared
 *           against $a0 with `bne`, so the first gate is an equality on a whole
 *           word; the front-side byte 0x80143F03 is read once with
 *           `lui`/`lbu` and tested with `bnez`, so the early exit is the zero
 *           test of that single load. The only call is the `jal` to 0x801F2C80
 *           and the 0x18-byte frame holds $ra at 0x10 with no saved argument,
 *           while the returned zero is the `addu $v0,$zero,$zero` before the
 *           `jr $ra`.
 *
 *           Shape: no payload under out/binaries holds this 64-byte body, and
 *           the same-address span of every other overlay carries a different
 *           body, so the statement order, literals and bindings here come from
 *           these bytes rather than from a sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 stateSharedFrontSide(u32 arg0) {
  if (arg0 == 0x4D8000) {
    if (D_80143F03 == 0) {
      stateSharedSetFlag3();
    }
  }
  return 0;
}
