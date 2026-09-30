#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D23A0
 * @behavior emits one free-size panel sprite for the requested icon kind:
 *           when the kind byte is 4 and the low seven bits of the stream index
 *           hint byte are neither 5 nor 0x0C, the kind is replaced with 0x0B;
 *           the drawing-mode primitive is then queued at the shared cursor
 *           with the texture page selected by GetGraphType (0x2E for graph
 *           types 1 and 2, 0x1E otherwise) before the sprite record is filled
 *           with the channel colour selected by the draw-state byte
 *           (0x808080 for 0, 0x303030 for 1 and 0x40/0x40/0x80 otherwise), the
 *           coordinates, the kind's four-byte texture record and the fixed
 *           0x28 by 0x30 size.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D23A0(s16 arg0, s16 arg1, u8 arg2, u8 arg3) {
  SPRT* sprite;
  u8* texture;
  s32 page;
  u32 kind;

  if (arg2 == 4) {
    u32 hint = D_80145024 & 0x7F;
    if (hint != 5 && hint != 0xC) {
      arg2 = 0xB;
    }
  }

  kind = arg2 & 0xFF;
  texture = &D_801D4058[kind * 4];

  if (GetGraphType() == 1) {
    page = 0x2E;
  } else if (GetGraphType() == 2) {
    page = 0x2E;
  } else {
    page = 0x1E;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, page, 0);
  func_8014E5A0(1, 0xC);

  sprite = (SPRT*)g_PrimCursor;
  SetSprt(sprite);

  if (arg3 == 0) {
    sprite->b0 = 0x80;
    sprite->g0 = 0x80;
    sprite->r0 = 0x80;
  } else if (arg3 == 1) {
    sprite->b0 = 0x30;
    sprite->g0 = 0x30;
    sprite->r0 = 0x30;
  } else {
    sprite->g0 = 0x40;
    sprite->r0 = 0x40;
    sprite->b0 = 0x80;
  }

  sprite->x0 = arg0;
  sprite->y0 = arg1;
  sprite->u0 = texture[0];
  sprite->v0 = texture[1];
  sprite->clut = (u16)(((texture[3] + 0x1E0) << 6) | (texture[2] >> 4));
  sprite->w = 0x28;
  sprite->h = 0x30;
  func_8014E5A0(1, 0x14);
}
