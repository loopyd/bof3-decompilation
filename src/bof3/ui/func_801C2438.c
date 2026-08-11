#include "bof3/ui/game00_internal.h"

/* @behavior marks 31 palette entries for the active work region, sets the
 * work-active flag, advances the palette stage serial, and clears its tail.
 * @source 0x801C2438
 * @status partial
 * @match 76.32
 * @residual Same-size allocator/commutative-order residual: loop row and entry
 * addresses use reversed addu operands/registers; tail reload uses v1 versus v0
 * and the final table-base addu operands are reversed after a 60s permuter run.
 */
void func_801C2438(void) {
  s32 i;
  struct GameWorkArea* work;
  struct GameWorkArea* loop_work;
  u8* entries;
  u8* row;
  u16* entry;
  u16 value;

  i = 0;
  entries = PSX_PTR(u8, 0x80039600u);
  work = g_game_work;
  loop_work = work;
  do {
    row = entries + (loop_work->field_05 << 6);
    entry = (u16*)(row + (i << 1));
    value = *entry;
    *entry = value | 0x8000u;
    i++;
  } while (i < 31);

  work = g_game_work;
  work->flags_00 |= 0x20u;
  work = g_game_work;
  i = work->field_05 << 5;
  paletteStageSerial = 1u;
  D_8003963E[i] = 0;
}
