#include "bof3/battle/battle03_internal.h"

/* @source 0x801E91CC
 * @behavior Reads two signed panel halfwords and calls func_801D750C.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void forwardPanelHalfwords(void) {
    func_801D750C(*(s16 *)&D_80148648[4], *(s16 *)&D_80148648[6]);
}
