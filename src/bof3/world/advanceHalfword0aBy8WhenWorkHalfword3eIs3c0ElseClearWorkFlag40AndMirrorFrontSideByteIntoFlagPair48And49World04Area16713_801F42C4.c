#include "bof3/bof3.h"

/* Front-side byte of the main executable, outside this overlay's payload. The
 * handler mirrors its value 1, 2 or 3 into the shared flag pair below, and this
 * target's map already binds the address. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Shared main-RAM flag bank of the main executable, outside this overlay's
 * payload. Its block bit setter and bit clear helper are the two calls below. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Front-end record pointer cell of the main executable, outside this overlay's
 * payload. Word 5 of the record it publishes is the halfword at +0x0A, the same
 * counter the exact sibling 0x801F43B8 advances by 3. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared block bit setter and bit clear helper of the main executable, outside
 * this overlay's payload. */
extern void func_8015B580(void* flag_bank, u8 bit_index); /* @source 0x8015B580 */
extern void func_8015B5A8(void* flag_bank, u8 bit_index); /* @source 0x8015B5A8 */

/* @source 0x801F42C4
 * @behavior Entry 11 of the 25-entry overlay handler table at 0x801F6830 of
 *           this payload, so no jal reaches this body. It reads the scratchpad
 *           work record once through the pointer cell 0x1F800044 and keeps the
 *           pointer in a saved register for the whole run. When the record
 *           halfword at +0x3E reads 0x3C0 the handler only adds 8 to word 5 of
 *           the front-end record published through 0x80148210, that is to the
 *           halfword at +0x0A. On every other value it clears flag 0x40 of the
 *           record byte at +0x00 and mirrors the front-side byte 0x80143F03
 *           into the shared flag bank 0x80144F28: value 1 raises bit 0x48 and
 *           clears bit 0x49, value 2 clears 0x48 and raises 0x49, value 3
 *           clears both, and any other value leaves the pair untouched. The
 *           front-side byte is read once and tested against 1, 2 and 3 in that
 *           order; the value-3 arm clears 0x48 and falls into the same
 *           bit-clear tail the value-1 arm reaches after its setter call. Takes
 *           no arguments and returns nothing; the 0x18-byte frame keeps $ra at
 *           +0x14 and $s0 at +0x10, and the first pointer load precedes the
 *           frame creation.
 *
 * Name evidence: the front-side byte is 0x80143F03, which the exact sibling
 * 0x801F43B8 of this target names the front-side byte and tests against the
 * same values, and the halfword it advances through 0x80148210[5] is the
 * published halfword 0x0A that family of siblings advances and lowers. The two
 * shared helpers are the bit setter 0x8015B580 and the bit clear helper
 * 0x8015B5A8 that the world siblings call with a bank base and a bit index. The
 * role of the 0x3C0 comparison value and of the flag bits 0x48 and 0x49 stays
 * structural: no lifted sibling in this repository names either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy8WhenWorkHalfword3eIs3c0ElseClearWorkFlag40AndMirrorFrontSideByteIntoFlagPair48And49World04Area16713_801F42C4(
    void) {
  u8* work;

  work = *(u8**)SPAD_ADDR(u8, 0x44u);
  if (FIELD_REF(s16, work, 0x3E) != 0x3C0) {
    work[0] &= 0xBF;
    if (D_80143F03 == 1) {
      func_8015B580(&D_80144F28, 0x48);
      func_8015B5A8(&D_80144F28, 0x49);
    } else if (D_80143F03 == 2) {
      func_8015B5A8(&D_80144F28, 0x48);
      func_8015B580(&D_80144F28, 0x49);
    } else if (D_80143F03 == 3) {
      func_8015B5A8(&D_80144F28, 0x48);
      func_8015B5A8(&D_80144F28, 0x49);
    }
  } else {
    D_80148210[5] += 8;
  }
}
