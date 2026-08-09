#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801AD6BC
 * @behavior Dispatches the handler selected by the work area's field_04.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801AD6BC(void) {
  GameEntry0DispatchSet handlers = D_80195F44;

  handlers.handlers[g_game_work->field_04]();
}
