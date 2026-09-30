#include "bof3/ui/game00_internal.h"

/* @behavior Decrements the scratch work-area countdown byte at 0x09 while it is
 *           nonzero; once the countdown has already reached zero it clears bit
 *           0x1C of the shared main-RAM flag bank at 0x80144F28 through the
 *           shared bit-clear service func_8015B5A8 and then clears the work-area
 *           flags through clearWorkFlags.
 * @source 0x8019AB5C
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearFlag1CAndWorkFlagsWhenCountdownExpires(void) {
  struct GameWorkArea* work;
  u8 count;

  work = g_game_work;
  count = work->pad_09[0];
  if (count == 0) {
    func_8015B5A8(&D_80144F28, 0x1C);
    clearWorkFlags();
  } else {
    work->pad_09[0] = count - 1;
  }
}
