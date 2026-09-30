#include "bof3/ui/game00_internal.h"

/* @behavior While the scratch work-area countdown byte at 0x09 is nonzero,
 * advances each of the three 16.16 panel accumulators at 0x801481E8 /
 * 0x801481EC / 0x801481F0 by its paired per-frame delta and commits the integer
 * part of every sum to the signed halfword panel coordinates at 0x801492D8 /
 * 0x801492DA / 0x801492DC, then decrements the countdown; once the countdown has
 * already reached zero it sets work byte 0x01 to 2 instead.
 * @source 0x8019AA44
 * @status exact
 * @match 100.00
 * @residual none
 * instructions, 184 bytes.
 * Seed note (cleared): the first seed interleaved each accumulator add with its
 * own coordinate store, which left the 68(a3) work pointer in `$a1` and the
 * sums in the wrong registers (32.61%, first=+0x0000). Grouping the three adds
 * before the three coordinate stores restored the `$a3` pointer and the
 * `a0`/`v0`/`v1` sum and `v0`/`a1`/`a2` delta assignment; the address-taken
 * accumulator closed the remaining address
 * materialization.
 */
void func_8019AA44(void) {
  struct GameWorkArea* work;
  s32* acc;

  work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
  if (work->pad_09[0] != 0) {
    acc = &D_801481E8;
    *acc += D_801481F8;
    D_801481EC += D_801481FC;
    D_801481F0 += D_80148200;
    D_801492D8 = *acc >> 16;
    D_801492DA = D_801481EC >> 16;
    D_801492DC = D_801481F0 >> 16;
    work->pad_09[0]--;
  } else {
    work->unk_01 = 2;
  }
}
