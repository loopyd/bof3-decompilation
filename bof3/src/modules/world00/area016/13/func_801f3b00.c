#include "internal.h"

/* does: draws the local status/icon strip set selected by the shared gate bits,
 * current world state helpers, and the marker-mask tables at `0x801f5194`.
 * @source: 0x801f3b00 FUN_801f3b00
 */
void func_801f3b00(s32 arg0, s32 arg1) {
  s16 x;
  s16 y;
  s8  helper_state;
  s32 i;
  u16 tpage;

  if ((WORLD00_AREA016_GLOBAL_BYTE_832E & 0x1bu) == 0u) {
    return;
  }

  tpage = 0x22cu;
  if (GetGraphType() != 1) {
    tpage = 0x9cu;
    if (GetGraphType() == 2) {
      tpage = 0x22cu;
    }
  }

  SetDrawMode((DR_MODE*)WORLD00_AREA016_PRIMITIVE_PTR, 0, 0, tpage, 0);
  func_8014e5a0(1u, 0x0cu);

  x = (s16)arg0;
  y = (s16)arg1;
  func_801f39d8(x, y, 0u);

  helper_state = func_80166cb0(WORLD00_AREA016_GLOBAL_HALF_930A,
                               WORLD00_AREA016_GLOBAL_HALF_930E);

  if ((((u8)(helper_state + 0x60)) < 2u) || (helper_state == (s8)0xae) ||
      ((WORLD00_AREA016_GLOBAL_HALF_625A & 0x1000u) != 0u)) {
    for (i = 0; i < 6; i += 1) {
      if ((WORLD00_AREA016_MARKER_TABLE[i].mask &
           WORLD00_AREA016_GLOBAL_HALF_5AB4) != 0u) {
        func_801f39d8((s16)(arg0 + 0x38), (s16)(arg1 + 8),
                      WORLD00_AREA016_MARKER_TABLE[i].field_02);
        break;
      }
    }
  } else {
    func_801f39d8((s16)(arg0 + 0x30), y, 1u);

    for (i = 0; i < 6; i += 1) {
      if ((WORLD00_AREA016_MARKER_TABLE[i].mask &
           WORLD00_AREA016_GLOBAL_HALF_5AB4) != 0u) {
        func_801f39d8(
            (s16)(arg0 + 0x38), (s16)(arg1 + 8),
            (u32)(WORLD00_AREA016_MARKER_TABLE[i].field_02 + 1u));
        break;
      }
    }
  }

  if (((u8)(helper_state + 0x60)) < 2u) {
    for (i = 0; i < 8; i += 1) {
      if ((WORLD00_AREA016_MARKER_TABLE[i].mask &
           WORLD00_AREA016_GLOBAL_HALF_5AC0) != 0u) {
        func_801f39d8((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      WORLD00_AREA016_MARKER_TABLE[i].field_02);
        break;
      }
    }
  } else {
    func_801f39d8((s16)(arg0 + 0x30), (s16)(arg1 + 0x10), 2u);

    for (i = 0; i < 8; i += 1) {
      if ((WORLD00_AREA016_MARKER_TABLE[i].mask &
           WORLD00_AREA016_GLOBAL_HALF_5AC0) != 0u) {
        func_801f39d8(
            (s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
            (u32)(WORLD00_AREA016_MARKER_TABLE[i].field_02 + 1u));
        break;
      }
    }
  }

  if ((func_801b6610(WORLD00_AREA016_GLOBAL_HALF_930A,
                     WORLD00_AREA016_GLOBAL_HALF_930E) != 0u) ||
      ((WORLD00_AREA016_GLOBAL_HALF_625A & 0x1000u) != 0u) ||
      (((WORLD00_AREA016_STREAM_HINT & 0x7fu) == 0x0cu) ||
       ((WORLD00_AREA016_GLOBAL_HALF_6258 & 0x4000u) != 0u))) {
    func_801f39d8((s16)(arg0 + 0x30), (s16)(arg1 + 0x18), 3u);
  }

  func_801f3ecc((s16)(arg0 + 0x18), (s16)(arg1 + 0x18));
}
