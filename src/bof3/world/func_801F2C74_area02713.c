#include "bof3/world/area02713_internal.h"

/* @behavior seeds the three local 0x98-byte world work records at 0x800e4800
 * with the fixed words 0x20000/0xc0000/0xb0000, the word 0x20000 and the
 * per-record turn angle (index * 0x1000) / 3, hands each record to the local
 * initializer at 0x801f304c, then resets scratch-work byte 9 and advances
 * scratch-work byte 1.
 * @source 0x801F2C74
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2C74(void) {
  World00Area027Work* work;
  u8 i;

  work = WORLD00_AREA027_WORK_BASE;
  i = 0u;
  do {
    work->unk_00 = 0x20000u;
    work->unk_04 = 0xC0000u;
    work->unk_08 = 0xB0000u;
    work->unk_10 = 0x20000u;
    work->unk_14 = (u16)((i << 12) / 3);
    func_801F304C(work);
    work = (World00Area027Work*)((u8*)work + 0x98u);
    i += 1u;
  } while (i < 3u);

  func_8015DF18(0x206);
  WORLD00_AREA027_SCRATCH_PTR[9] = 0x40u;
  WORLD00_AREA027_SCRATCH_PTR[1] += 1u;
}
