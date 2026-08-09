#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801AD46C
 * @behavior Dispatches through a five-entry handler table using work field 04.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801AD46C(void) {
  GameEntry0HandlerSet handlers = D_80195F10;

  handlers.handlers[*((u8*)g_game_work + 4)]();
}
