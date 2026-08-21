#include "bof3/world/area01613_internal.h"

/* @source 0x801F3B00
 * @behavior draws the AREA016 marker sprite layers selected by field state and
 * appends the transformed background panel.
 * @status partial
 * @match 78.19
 * @residual size and marker-loop control-flow scheduling differ
 */
void func_801F3B00(s16 arg0, s16 arg1) {
  s32 marker;
  s32 i;

  if (!(D_8014832E & 0x1b)) {
    return;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0,
              GetGraphType() == 1 ? 0x22c
                                  : (GetGraphType() == 2 ? 0x22c : 0x9c),
              0);
  func_8014E5A0(1, 0x0c);
  emitSemiTransparentSprite((s16)arg0, (s16)arg1, 0);

  marker = func_80166CB0(PSX_REF(s16, 0x8014930au),
                         PSX_REF(s16, 0x8014930eu));
  if (((u32)((marker + 0x60) & 0xff) >= 2) && ((marker & 0xff) != 0xae) &&
      !(PSX_REF(u16, 0x8014625au) & 0x1000)) {
    emitSemiTransparentSprite((s16)(arg0 + 0x30), (s16)arg1, 1);
    for (i = 0; i < 6; i++) {
      if (WORLD00_AREA016_MARKER_TABLE[i].mask & D_80145AB4) {
        emitSemiTransparentSprite(
            (s16)(arg0 + 0x38), (s16)(arg1 + 8),
            (u8)(WORLD00_AREA016_MARKER_TABLE[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 6; i++) {
      if (WORLD00_AREA016_MARKER_TABLE[i].mask & D_80145AB4) {
        emitSemiTransparentSprite((s16)(arg0 + 0x38), (s16)(arg1 + 8),
                                  WORLD00_AREA016_MARKER_TABLE[i].field_02);
        break;
      }
    }
  }

  if ((u32)((marker + 0x60) & 0xff) >= 2) {
    emitSemiTransparentSprite((s16)(arg0 + 0x30), (s16)(arg1 + 0x10), 2);
    for (i = 0; i < 8; i++) {
      if (WORLD00_AREA016_MARKER_TABLE[i].mask & D_80145AC0) {
        emitSemiTransparentSprite(
            (s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
            (u8)(WORLD00_AREA016_MARKER_TABLE[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 8; i++) {
      if (WORLD00_AREA016_MARKER_TABLE[i].mask & D_80145AC0) {
        emitSemiTransparentSprite((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                                  WORLD00_AREA016_MARKER_TABLE[i].field_02);
        break;
      }
    }
  }

  if ((func_801B6610(PSX_REF(s16, 0x8014930au),
                      PSX_REF(s16, 0x8014930eu)) & 0xff) ||
      (PSX_REF(u16, 0x8014625au) & 0x1000) ||
      ((PSX_REF(u8, 0x80145024u) & 0x7f) == 0x0c) ||
      (D_80146258 & 0x4000)) {
    emitSemiTransparentSprite((s16)(arg0 + 0x30), (s16)(arg1 + 0x18), 3);
  }
  appendTransformedG4Panel((s16)(arg0 + 0x18), (s16)(arg1 + 0x18));
}
