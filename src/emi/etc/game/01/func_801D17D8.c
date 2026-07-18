#include "internal.h"

#define GAME_FRONT_PRIMITIVE       PSX_PTR(volatile u8, 0x8014598cu)
#define GAME_FRONT_GLYPH_GEOMETRY  PSX_PTR(const volatile u16, 0x801d1c6cu)
#define GAME_FRONT_GEOMETRY_STRIDE 5

/* @behavior initializes the shared frontend primitive and applies one indexed
 * glyph geometry record before returning the primitive to the caller.
 * @source 0x801D17D8
 */
u8* func_801D17D8(s32 x, s32 y, s32 glyph, s32 palette, u8 flags) {
  u8* primitive;
  s32 geometry_offset;

  primitive = (u8*)GAME_FRONT_PRIMITIVE;
  func_8017AA1C(primitive);
  func_8017A904(primitive, flags);

  primitive[4] = 0x80u;
  primitive[5] = 0x80u;
  primitive[6] = 0x80u;
  geometry_offset = (u8)glyph * GAME_FRONT_GEOMETRY_STRIDE;
  *(s16*)(primitive + 8) = x;
  *(s16*)(primitive + 10) = y;
  primitive[12] = GAME_FRONT_GLYPH_GEOMETRY[geometry_offset + 0];
  primitive[13] = GAME_FRONT_GLYPH_GEOMETRY[geometry_offset + 1];
  *(u16*)(primitive + 16) = GAME_FRONT_GLYPH_GEOMETRY[geometry_offset + 2];
  *(u16*)(primitive + 18) = GAME_FRONT_GLYPH_GEOMETRY[geometry_offset + 3];
  *(u16*)(primitive + 14) = GAME_FRONT_GLYPH_GEOMETRY[geometry_offset + 4] << 6;
  func_8014E5A0((u8)palette, 20);
  return primitive;
}
