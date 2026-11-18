#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 overlay per-frame entry: publishes the shared active work
 * area through func_80199440, dispatches the area work records through
 * dispatchActiveWorkRecordsAndUpdateLevelsWorld00Area03004 and then runs the shared frame-service chain func_801527E4,
 * func_801BDAB8, func_801E09B4, func_8019A0E4 and func_8014BA54 in that order
 * (runArea030FrameServices is that same five-call chain on its own). The
 * GAME.EMI entry-0 front-end substate-4 handler func_801984E8 invokes it once per
 * frame (direct jal at 0x80198578) after installing the overlay draw-move
 * primitive and before it advances the D_80143B92 sub-state counter.
 * @source 0x801E0BD0
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateArea030OverlayFrame(void) {
    func_80199440();
    dispatchActiveWorkRecordsAndUpdateLevelsWorld00Area03004();
    func_801527E4();
    func_801BDAB8();
    func_801E09B4();
    func_8019A0E4();
    func_8014BA54();
}
