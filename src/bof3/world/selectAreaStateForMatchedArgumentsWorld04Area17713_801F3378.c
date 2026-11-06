#include "bof3/bof3.h"

extern u8 D_80145E98; /* @source 0x80145E98 @kind data */
extern s8 D_80146870; /* @source 0x80146870 @kind data */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* @source 0x801F3378
 * @behavior Selects the shared area state byte 0x801448EB from the signed
 *   scenario progress byte 0x80146870 once the overlay request matches: the
 *   function returns 0 unless the first two zero-extended u16 arguments read
 *   0x3F and 0xE and the leading byte of the main-RAM game-mode record
 *   0x80145E98 reads 1. On a match it stores 6 when the progress byte is below
 *   8 and 0x1A otherwise, then returns 1. No calls, no frame, no other stores.
 *
 * Name evidence: 0x801448EB is the state selector that other overlays dispatch
 *   through their handler tables (etc/game/00 reads it as an s8 table index,
 *   world01/area044/13 treats it as the shared area task state byte), and the
 *   0x80146870/0x80145E98 roles come from their declarations in
 *   include/bof3/world/area01613_internal.h and
 *   include/bof3/ui/shop00_internal.h. The two arguments carry no evidenced
 *   role, so the name keeps them structural.
 *
 * Shape: the 108-byte range is byte-identical across the world04 area
 *   overlays (payload span 0x778..0x7E4); the area176 member carries an exact
 *   source for the same bytes, and this body copies its statement shape. The
 *   decisive part is the third guard written as a positive block whose body
 *   holds the store select and its own `return 1`, with `return 0` after the
 *   block: that keeps the at-least-8 arm inline and puts the below-8 arm out
 *   of line behind `j` with an unfilled branch delay slot. An early `return 0`
 *   guard followed by a function-level select merges the two arms into one
 *   store block and fills that delay slot, eight bytes short.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectAreaStateForMatchedArgumentsWorld04Area17713_801F3378(u16 arg0, u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 == 1) {
    if (D_80146870 >= 8) {
      D_801448EB = 0x1A;
    } else {
      D_801448EB = 6;
    }
    return 1;
  }
  return 0;
}
