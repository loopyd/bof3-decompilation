#include "bof3/ui/game00_internal.h"

extern u32 func_8014D8D4(u8 arg0);

/*
 * @source 0x801C4D1C
 * @behavior Shared work-area select service: forwards the route-index-selected
 * selection effect id to the shared EXE-side helper func_8014D8D4. The id is
 * 0x3C when the scratchpad work area route index (g_game_work->route_index_08)
 * reads 7 and 0x3D otherwise. Both ids are constants, so the original collapses
 * the choice into one call: the 0x3D arm is materialised in $a0 in the branch
 * delay slot of the route-index test and the 0x3C arm on the fall-through.
 * @status partial
 * @match 93.33
 * @residual one instruction position: the original issues the scratchpad work
 * area load (lui/lw 0x1F800044) before the frame setup (addiu sp,sp,-0x18) and
 * this build issues it after it, so 14/15 instructions match at identical size
 * (60 bytes), first=+0x0000 addiu sp,sp,-24. Same-size load-order (scheduling)
 * residual: invariant across eight measured clean-C shapes. The four shapes
 * tried in the repair round exercised the pointer-cell spelling lever
 * (SPAD_PTR_SLOT constant-address load as a leading statement and as a
 * declaration initializer, PSX_REF(SPAD_BASE + 0x44u) with the route byte read
 * into a local, and the symbolic g_game_work load) and all stayed at 14/15 with
 * the same first difference, so the spelling is not the lever. A shape that
 * yields the target order does exist: the exact same-target siblings
 * func_8019A8B4 and func_8019AA44 hoist the same constant-address slot load
 * above the identical 24-byte frame, as does the symbolic load in exact
 * func_8019AAFC. Smallest missing evidence: the source shape (not the
 * declaration form) that schedules this slot load ahead of the frame setup for
 * this branch-select body; the byte-identical head in the still-asm sibling
 * func_801B73F4 depends on that same shape.
 */
void func_801C4D1C(void)
{
    u8 effectId;

    effectId = 0x3Du;
    if (g_game_work->route_index_08 == 7) {
        effectId = 0x3Cu;
    }
    func_8014D8D4(effectId);
}
