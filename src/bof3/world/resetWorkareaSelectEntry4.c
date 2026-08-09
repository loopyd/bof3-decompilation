#include "bof3/world/area02414_internal.h"
#include "game/workarea.h"

/* @source 0x801F2DD8
 * @behavior entry-4 handler of the overlay entry table (D_801F4200):
 * calls the shared work-area reset helper.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
WORKAREA_RESET(resetWorkareaSelectEntry4)
