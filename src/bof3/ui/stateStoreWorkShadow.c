#include "bof3/ui/game00_internal.h"

/* @source 0x8019AD70
 * @behavior copies the work-area word at offset 0x0C into the u16 shadow at
 * D_8014932C, then runs the work-coordinate save helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStoreWorkShadow(void) {
  D_8014932C = g_game_work->field_0C;
  stateSaveWorkCoordinates();
}
