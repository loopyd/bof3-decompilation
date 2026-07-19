#include "internal.h"

/* @source 0x801F2DD8
 * @behavior resets the scratchpad game work-area header by delegating to
 * func_80196070 (clears flags_00, unk_01, flags_02, pad_03[0..1]). This is a
 * thin tail-call wrapper with no extra logic.
 */
#define WORKAREA_RESET_FUNC func_801F2DD8
#include "shared/game/workarea_reset.inc"
