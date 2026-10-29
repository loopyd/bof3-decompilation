#include "bof3/bof3.h"

extern s32 D_801468BC;

/* @source 0x801F2ED0
 * @behavior Overlay step: retreats the target-local 32-bit word counter at
 * 0x801468BC by 0x800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter3CloneWorld03Area13113_801F2ED0(void) {
  s32* counter;
  s32 value;

  counter = &D_801468BC;
  value = *counter;
  value -= 0x800;
  *counter = value;
}
