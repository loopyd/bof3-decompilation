#include "bof3/world/area02813_internal.h"
#include "game/workarea.h"

/* @source 0x801F2D1C
 * @behavior mode-2 handler of the entry mode dispatch table (D_801F34A8):
 * calls the shared work-area reset helper.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
WORKAREA_RESET(resetWorkareaSelectMode2)
