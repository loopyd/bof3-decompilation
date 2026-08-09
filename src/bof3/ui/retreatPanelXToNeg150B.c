#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2E30
 * @behavior Decrements panel task x by 0x20, clamping at -0x96 and clearing
 *           state when the clamp fires.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void retreatPanelXToNeg150B(void) {
  PanelTask* task = D_80148648;
  u16        val = task->x - 0x20;
  task->x = val;
  if ((s16)val < -0x96) {
    s16 clamped = -0x96;
    task->x = clamped;
    task->state = 0;
  }
}
