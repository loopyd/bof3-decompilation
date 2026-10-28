#include "bof3/bof3.h"
#include "world/work_state.h"

/* @source 0x801F3478
 * @behavior clears scratch-state offsets 0x09 and 0x0b, then increments 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_RESET_ADVANCE_STATE(resetAdvanceScratchStateCloneWorld03Area12114_801F3478)

/* @source 0x801F3418
 * @behavior Writes 0xA0 to the scratchpad halfword at pointer-slot 0x44 + 0x2E and 0x50 to the halfword at + 0x30.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_WORK_HALFWORDS_A0_50(setWorkHalfwordsWorld03Area12114_801F3418)
