#include "bof3/world/area01613_internal.h"
#include "world/work_state.h"

/* @source 0x801F3460
 * @behavior clears scratch-state offsets 0x09 and 0x0b, then increments 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_RESET_ADVANCE_STATE(resetAdvanceScratchState)
