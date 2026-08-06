#include "internal.h"
#include "game/workarea.h"

/* @source 0x801F2EE4
 * @behavior slot-4 entry of the local handler pointer table (T_801F3F6C):
 * calls the shared work-area reset helper. Slot semantics beyond the
 * table membership are unproven.
 */
WORKAREA_RESET(resetWorkarea)
