#include "internal.h"

/* @source 0x80199638
 * @behavior advances the panel x coordinate by 32 and stops at 80.
 */
void func_80199638(void) {
  PanelTask* panel;

  panel = D_80148648;
  panel->x += 32;
  if ((s16)panel->x > 80) {
    panel->x = 80;
    panel->state = 0;
  }
}
