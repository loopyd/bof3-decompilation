#include "internal.h"

/* @behavior initializes the shared frontend primitive and applies one indexed
 * glyph geometry record before returning the primitive to the caller.
 * @source 0x801D17D8
 */
u8* game_front_draw_glyph(s32 x, s32 y, s32 glyph, s32 palette, u8 flags) {
  u8* primitive;
  s32 glyph_index;

  primitive = (u8*)D_8014598C;
  SetSprt((SPRT*)primitive);
  SetSemiTrans((void*)primitive, flags);

  primitive[4] = 0x80u;
  primitive[5] = 0x80u;
  primitive[6] = 0x80u;
  glyph_index = (u8)glyph;
  *(s16*)(primitive + 8) = x;
  *(s16*)(primitive + 10) = y;
  primitive[12] = D_801D1C6C[glyph_index].unk_0;
  primitive[13] = D_801D1C6C[glyph_index].unk_2;
  *(u16*)(primitive + 16) = D_801D1C6C[glyph_index].unk_4;
  *(u16*)(primitive + 18) = D_801D1C6C[glyph_index].unk_6;
  *(u16*)(primitive + 14) = D_801D1C6C[glyph_index].unk_8 << 6;
  func_8014E5A0((u8)palette, 20);
  return primitive;
}
