#include "bof3/context.h"
#include "bof3/core/slus_internal.h"

extern void appendRenderPrim(u32 arg0, u32 arg1);

extern SPRT_8* g_PrimCursor;

/* The string is read-only. A mutable local view preserves the original's
 * repeated cursor reads across sprite stores without changing the public API.
 * @behavior emits one 8x8 glyph sprite for each non-space byte of the string,
 * advancing eight pixels per byte and wrapping on newline.
 * @source 0x80150098
 * @status partial
 * @match 95.18
 * @residual 79/83 instructions at the original 332-byte size. Prologue copy
 * order and loop register allocation differ. The signed divide/modulo-by-32
 * expansion and repeated glyph reads follow the original.
 */
void drawGlyphString8x8(s16 x, s16 y, u32 clut, const u8* input) {
  u8*     text;
  SPRT_8* primitive;
  s16     start_x;
  u16     primitive_clut;

  text = (u8*)input;
  func_8014FC00(0);

  primitive = g_PrimCursor;
  clut &= 0x3fu;
  primitive_clut = (u16)(clut | 0x7800u);
  start_x = x;

  do {
    if ((*text) == 10) {
      x = start_x;
      y += 8;
    } else if (((*text) != ' ') && ((*text) != 0)) {
      primitive->clut = primitive_clut;
      primitive->r0 = 0x80;
      primitive->g0 = 0x80;
      primitive->b0 = 0x80;

      /* Signed (glyph - 0x20) % 32 and / 32, expanded by hand so the emitted
       * sign fix-up matches the original's bgez/addiu -1 sequence. */
      {
        s32 glyph_byte_u = (s32)(*text);
        s32 glyph_row_u = glyph_byte_u - 0x20;
        s32 glyph_index_u = glyph_row_u;

        if (glyph_index_u < 0) {
          glyph_index_u = glyph_byte_u - 1;
        }
        primitive->u0 =
            (u8)((glyph_row_u - ((glyph_index_u >> 5) << 5)) << 3);
      }
      {
        s32 glyph_byte_v = (s32)(*text);
        s32 glyph_index_v = glyph_byte_v - 0x20;

        if (glyph_index_v < 0) {
          glyph_index_v = glyph_byte_v - 1;
        }
        primitive->v0 = (u8)((glyph_index_v >> 5) << 3);
      }
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
