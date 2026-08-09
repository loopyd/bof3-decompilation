#include "bof3/ui/game00_internal.h"
#include "game/workarea.h"

/* @source 0x8019EAD4
 * @behavior calls the shared work-area reset helper.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
WORKAREA_RESET(resetWork2)
