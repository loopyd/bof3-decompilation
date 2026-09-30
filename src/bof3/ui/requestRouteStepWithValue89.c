#include "bof3/ui/game00_internal.h"

/*
 * @source 0x801C3790
 * @behavior Gated request wrapper around func_801C364C: when bit 0x400 of the
 * world flags word D_80146258 is set it reports 0, otherwise it runs the
 * route-step positioning attempt with the tile-query value 0x89 and the step
 * byte 2 (the caller byte is masked to eight bits) and reports whether that
 * attempt succeeded.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestRouteStepWithValue89(s32 arg0)
{
    if ((D_80146258 & 0x400) == 0) {
        return (u8)func_801C364C(0x89, 2, (u8)arg0) != 0;
    }
    return 0;
}
