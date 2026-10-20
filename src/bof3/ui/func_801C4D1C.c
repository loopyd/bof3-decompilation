#include "bof3/ui/game00_internal.h"

extern u32 func_8014D8D4(u8 arg0);

/*
 * @source 0x801C4D1C
 * @behavior Shared work-area select service: forwards the route-index-selected
 * selection effect id to the shared EXE-side helper func_8014D8D4. The id is
 * 0x3C when the scratchpad work area route index (work->route_index_08) reads 7
 * and 0x3D otherwise. Both ids are constants, so the original collapses the
 * choice into one call: the 0x3D arm is materialised in $a0 in the branch delay
 * slot of the route-index test and the 0x3C arm on the taken path. The selection
 * is written as two identical calls in the if/else arms, which the compiler
 * cross-jumps into the original's single jal - the same shape as the exact
 * sibling func_801B73F4. That two-call shape is also what
 * schedules the scratchpad pointer-cell load above the frame setup; the
 * single-call result-local form issued `addiu sp,sp,-0x18` first and stalled at
 * 14/15 instructions.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801C4D1C(void)
{
    struct GameWorkArea* work;

    work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
    if (work->route_index_08 == 7) {
        func_8014D8D4(0x3Cu);
    } else {
        func_8014D8D4(0x3Du);
    }
}
