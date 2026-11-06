#include "bof3/bof3.h"

extern u8 D_80145E98; /* @source 0x80145E98 @kind data */
extern u8 D_80146865; /* @source 0x80146865 @kind data */

/* @source 0x801F3218
 * @behavior Publishes the leading mode byte of the current game-mode record
 * 0x80145E98 into the shared area status byte 0x80146865. The load folds its
 * displacement into the lui base, the store re-materialises the destination
 * base in $at, and neither access takes a local, so the original translation
 * unit declared no value temporary. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x80145E98 is the leading byte of the game-mode record
 * declared as GameModeRecord[] at the same address in
 * include/bof3/ui/game00_internal.h, and 0x80146865 is the shared area status
 * byte that the area-mode handlers test. The two exact siblings
 * copyModeByteWorld03Area13313_801F2DBC and
 * copyModeByteWorld03Area13113_801F2EB8 carry the same role wording for their
 * own overlays.
 *
 * Shape: the 24-byte range is byte-identical to those two already-exact
 * cross-target sources (24-byte cmp of the payload spans), so the body is a
 * shape copy of that sibling with this function's own source address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyModeByteWorld02Area09413_801F3218(void) {
  D_80146865 = D_80145E98;
}
