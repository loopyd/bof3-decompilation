#include "bof3/context.h"
#include "bof3/core/slus_internal.h"

extern void appendRenderPrim(u32 arg0, u32 arg1);

extern SPRT_8* g_PrimCursor;

/* @behavior emits one 8x8 glyph sprite for each non-space byte in the string,
 * advancing eight pixels per byte and wrapping on newline.
 * @source 0x80150098
 * @status partial
 * @match 51.81
 * @residual non-exact live audit: 43/83 instructions; 332 original bytes versus 308 current.
 */
void func_80150098(s16 x, s16 y, u32 clut, const u8* text) {
  SPRT_8* primitive;
  s16     start_x;
  s32     char_index;
  s32     text_char;
  u16     primitive_clut;

  func_8014FC00(0);

  clut &= 0x3fu;
  primitive_clut = (u16)(clut | 0x7800u);
  start_x = x;
  primitive = g_PrimCursor;

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
      appendRenderPrim(1, 0x10);
      primitive = g_PrimCursor;
    }

    text += 1;
    x += 8;
  } while ((*text) != 0);
}
