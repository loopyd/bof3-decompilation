#include "internal.h"

/* @source 0x801D2520
 * @behavior Initializes an icon/sprite primitive at the current PsyQ primitive
 * cursor (D_8014598C): resets its shape via SetSprt8, writes the six caller
 * parameters into the primitive's offset fields (s16 position at 0x8/0xA, s8
 * scale*8 at 0xC/0xD, u16 param at 0xE, u8 value at 0x4/0x5/0x6), clears
 * semi-transparency via SetSemiTrans (flag 0), then appends the 0x10-byte
 * primitive to OT index 1 via func_8014E5A0.
 */
#define BOF3_INIT_ICON_PRIM_FUNC func_801D2520
#include "shared/ui/init_icon_prim.inc"
