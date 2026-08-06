#include "internal.h"

/* @source 0x801E91CC
 * @behavior Reads two signed panel halfwords and calls func_801D750C.
 */
void battle03_panel_forward_halfwords(void) {
    func_801D750C(*(s16 *)&D_80148648[4], *(s16 *)&D_80148648[6]);
}
