#include "internal.h"

#include "bof3/context.h"

void func_8014e5a0(u32 arg0, u32 arg1);

extern SPRT_8* DAT_8014598c;

/* does: emits one 8x8 glyph sprite for each non-space byte in the string,
 * advancing eight pixels per byte and wrapping on newline.
 * @source: 0x80150098 FUN_80150098
 */
void func_80150098(s16 x, s16 y, u32 clut, const u8* text) {
  SPRT_8* primitive;
  s16     start_x;
  s32     char_index;
  s32     text_char;
  u16     primitive_clut;

  func_8014fc00(0);

  clut &= 0x3fu;
  primitive_clut = (u16)(clut | 0x7800u);
  start_x = x;
  primitive = DAT_8014598c;

  do {
    if ((*text) == 10) {
      x = start_x;
      y += 8;
    } else if (((*text) != ' ') && ((*text) != 0)) {
      primitive->clut = primitive_clut;
      primitive->r0 = 0x80;
      primitive->g0 = 0x80;
      primitive->b0 = 0x80;

      text_char = (s32)(*text);
      char_index = text_char - 0x20;
      if (char_index < 0) {
        char_index = text_char - 1;
      }

      primitive->u0 =
          (u8)(((text_char - 0x20) - (((text_char - 1) >> 5) << 5)) << 3);
      primitive->v0 = (u8)((char_index >> 5) << 3);
      primitive->x0 = x;
      primitive->y0 = y;

      SetSprt8(primitive);
      SetSemiTrans(primitive, 1);
      func_8014e5a0(1, 0x10);
      primitive = DAT_8014598c;
    }

    text += 1;
    x += 8;
  } while ((*text) != 0);
}
