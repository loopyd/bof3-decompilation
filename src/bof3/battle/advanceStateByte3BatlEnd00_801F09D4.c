#include "bof3/bof3.h"

extern u8 *D_80148648;

/* @source 0x801F09D4
 * @behavior Advances the shared panel state byte of the battle-end panel: reads
 *           the object published at 0x80148648, increments its byte +0x03 (the
 *           PanelTask state field of include/panel/task.h) and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 * Byte-identical twin of advanceStateByte3BatlEnd00_801F07C0 in this target;
 * the state byte indexed here is the axis dispatched by func_801F097C.
 */
void advanceStateByte3BatlEnd00_801F09D4(void) {
  ((u8 *)D_80148648)[3]++;
}
