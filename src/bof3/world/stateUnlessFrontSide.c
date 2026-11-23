#include "bof3/bof3.h"

/* Front-side byte of the main executable, outside this overlay's payload. The
 * neighbouring world04 area167 lift reads the same byte as that front-side
 * cursor and mirrors its values 1, 2 and 3 into a shared flag pair, and this
 * target's map already binds the address. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Main-RAM flag bank of the main executable, outside this overlay's payload.
 * The neighbouring world04 area167 lift hands the same bank to the shared block
 * bit setter 0x8015B580 and to the bit clear helper below. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared block flag-bit clear helper of the main executable, outside this
 * overlay's payload. */
extern void func_8015B5A8(void* flag_bank, u8 bit_index); /* @source 0x8015B5A8 */

/* @source 0x801F3764
 * @behavior Handler of this payload's handler table at 0x801F3F5C, whose word
 *           138 (0x801F4184) holds 0x801F3764: no jal reaches this body, and
 *           the overlay dispatcher 0x801F2C5C indexes that table by the state
 *           byte at +0x02 of the work record published at the scratchpad
 *           pointer cell 0x1F800044. It reads the shared front-side byte
 *           D_80143F03 once and, on every value other than 3, clears flag 0x74
 *           of the main-RAM flag bank D_80144F28 through the shared bit clear
 *           helper func_8015B5A8; on the value 3 it clears no flag. Takes no
 *           arguments and returns nothing, and its 0x18-byte frame only holds
 *           the return address of that one call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateUnlessFrontSide(void) {
  if (D_80143F03 != 3) {
    func_8015B5A8(&D_80144F28, 0x74);
  }
}
