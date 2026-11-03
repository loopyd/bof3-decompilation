#include "bof3/world/area03004_internal.h"

/* @behavior slides the AREA030 panel in and out by 45 pixels per remaining step of
 * the work-record countdown byte 9. A zero countdown arms byte 0x0B and steps the
 * dispatch index, four when byte 7 has bit 0 set and otherwise one past its current
 * value. While byte 0x0B stays clear the panel rectangle is inset by 18 - 16 * count
 * and the mask argument is zero; once it is set the fixed rectangle is used with the
 * mask built from bit 0 shifted by scratch byte 6 while the shared byte 0x80145E93
 * holds three or more. The panel row is then drawn at 16 - 45 * count and the footer
 * at 164 + 45 * count, through func_801D7EB0 or func_801D8364 depending on bit 0 of
 * work byte 7.
 * @source 0x801D4378
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D4378(void) {
  u8* first;
  u8* cursor;
  u8* maskCursor;
  u8 count;
  u32 mask;

  first = (u8*)D_1F800044;
  first[9u] = (u8)(first[9u] - 1u);

  cursor = (u8*)D_1F800044;
  if (cursor[9u] == 0u) {
    cursor[11u] = 1u;
    cursor = (u8*)D_1F800044;
    if ((cursor[7u] & 1u) == 0u) {
      cursor[1u] = (u8)(cursor[1u] + 1u);
    } else {
      cursor[1u] = 4u;
    }
    cursor = (u8*)D_1F800044;
  }

  if (cursor[11u] == 0u) {
    count = cursor[9u];
    func_801D9534(20, (u16)((18 - (count << 4)) & 0xFFFEu), 280, 19, 0);
    count = ((u8*)D_1F800044)[9u];
    func_801D7C10((u16)(88 + count * 45), 40, 0);
  } else {
    func_801D9534(20, 18, 280, 19, 0);
    mask = 0u;
    if (D_80145E93 >= 3u) {
      maskCursor = (u8*)D_1F800044;
      mask = (u8)(1u << maskCursor[6u]);
    }
    func_801D7C10(88, 40, mask);
  }

  count = ((u8*)D_1F800044)[9u];
  func_801D88E0((s16)(16 - count * 45), 64);

  cursor = (u8*)D_1F800044;
  if ((cursor[7u] & 1u) == 0u) {
    func_801D7EB0((s16)(164 + cursor[9u] * 45), 64);
  } else {
    func_801D8364((s16)(164 + cursor[9u] * 45), 64);
  }
}
