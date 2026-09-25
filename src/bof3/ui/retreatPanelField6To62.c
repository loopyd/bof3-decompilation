#include "bof3/ui/shop00_internal.h"
#include "base/compiler.h"

/* @source 0x801E2D1C
 * @behavior subtracts 0x10 from the panel task field at offset 6, raises values below 0x3E, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatPanelField6To62(void) {
  PanelTask* task_root;

  task_root = D_80148648;
  task_root->field_06 = (u16)(task_root->field_06 - 0x10);
  if ((s16)task_root->field_06 < 0x3E) {
    task_root->field_06 = 0x3E;
    task_root->state = 0;
  }
}
