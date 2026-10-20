#include "bof3/ui/game00_internal.h"

/* @behavior Marks 31 palette entries for the active work region, sets the
 * work-active flag, advances the palette stage serial, and clears its tail.
 * @source 0x801C2438
 * @status exact
 * @match 100.00
 * @residual none
 * The row base stays a local `u32` and the entry address is built as
 * `(i << 1) + row`, so both additions keep the original's integer-first
 * operand order; the tail clears the row's trailing halfword through the
 * fixed-address form, which is what makes the assembler emit `addu at,index,at`
 * instead of the named-symbol order.
 */
void func_801C2438(void) {
  s32 i;
  struct GameWorkArea* work;
  struct GameWorkArea* loop_work;
  u32 base;
  u16* entry;
  u16 value;

  i = 0;
  base = 0x80039600u;
  work = g_game_work;
  loop_work = work;
  do {
    entry =
        (u16*)((u32)(i << 1) + (((u32)loop_work->field_05 << 6) + base));
    value = *entry;
    *entry = value | 0x8000u;
    i++;
  } while (i < 31);

  work = g_game_work;
  work->flags_00 |= 0x20u;
  i = g_game_work->field_05 << 5;
  paletteStageSerial = 1u;
  PSX_PTR(u16, 0x8003963Eu)[i] = 0;
}
