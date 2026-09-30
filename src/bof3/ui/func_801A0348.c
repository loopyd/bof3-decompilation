#include "bof3/ui/game00_internal.h"

/* The call at 0x801A0364 passes both coordinates as sign-extended 16-bit
 * values, so the declaration visible in this translation unit takes 16-bit
 * coordinates; the callee's own translation unit declares the same two
 * parameters as bytes and the callee narrows them itself. */
u8* func_801A0490(s16 x, s16 y, u16 index);

/* @behavior returns byte 5 of the eight-byte bounding-box record that contains
 * the point (arg0, arg1) in the box list of the pending world id D_80143F00,
 * i.e. the byte behind the func_801A0490 pointer for that point; the caller
 * func_8019FBF8 publishes the result into D_80143F03.
 * @source 0x801A0348
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801A0348(s16 arg0, s16 arg1) {
  return func_801A0490(arg0, arg1, D_80143F00)[5];
}
