#include "bof3/ui/game00_internal.h"

/* @behavior reads one pixel from the 4bpp bitmap based at 0x80032000: a column
 * at or beyond the D_80104000 width or a row at or beyond the D_80104001
 * height yields 0, otherwise the pixel index (column + width * row) selects the
 * bitmap byte at index / 2 and an odd index returns its low nibble while an
 * even index returns its high nibble.
 * @source 0x801C6FD4
 * @status exact
 * @match 100.00
 * @residual none
 * The bitmap base stays an explicit fixed address: the original encodes it as a
 * full 32-bit constant (assembling to `addu at,index,at`), which a declared
 * symbol reference would not reproduce.
 */
u8 func_801C6FD4(s16 arg0, s16 arg1) {
  u16 index;
  u16 offset;

  if ((u16)arg0 >= D_80104000) {
    return 0;
  }
  if ((u16)arg1 >= D_80104001) {
    return 0;
  }

  index = arg0 + D_80104000 * arg1;
  offset = index >> 1;
  if (index & 1) {
    return PSX_PTR(u8, 0x80032000u)[offset] & 0xF;
  }
  return PSX_PTR(u8, 0x80032000u)[offset] >> 4;
}
