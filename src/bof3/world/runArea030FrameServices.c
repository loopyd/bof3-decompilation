#include "bof3/world/area03004_internal.h"

/* @behavior runs the shared frame-service chain of the AREA030 overlay frame:
 * func_801527E4, func_801BDAB8, func_801E09B4, func_8019A0E4 and func_8014BA54
 * are called in that order and the function returns. The main-executable
 * front-end substate-4 handler func_801985C8 calls it inside both of its wait
 * loops (direct jals at 0x80198614 and 0x80198694, each loop iteration followed
 * by func_8014B87C(1)); updateArea030OverlayFrame runs the same five services
 * after its own work-area publish and 0x1E-record dispatch.
 * @source 0x801E0C40
 * @status exact
 * @match 100.00
 * @residual none
 */
void runArea030FrameServices(void) {
    func_801527E4();
    func_801BDAB8();
    func_801E09B4();
    func_8019A0E4();
    func_8014BA54();
}
