#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0FBC */
extern void func_801F0FBC(void);

/* @source 0x801EEFFC
 * @behavior Decrements work byte 9 at byte width. When it reaches zero,
 *           reloads the scratchpad work pointer and advances state byte 1;
 *           otherwise calls the target-local helper at 0x801F0FBC. The entry
 *           is published at payload offset 0x14 in the leading handler table.
 *           Takes no arguments and returns no value. The helper's role remains
 *           address-anchored. The named pointer cell preserves the original
 *           reload after the potentially aliasing byte store. Statement shape
 *           follows the byte-9 countdown in magic009/03 at 0x801EF01C,
 *           with this span's additional nonzero call arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AdvanceWorkByte1OnZeroOtherwiseCall801F0FBCBmagicMagic11403_801EEFFC(void) {
  if (--g_battle_work[9] == 0) {
    g_battle_work[1]++;
  } else {
    func_801F0FBC();
  }
}
