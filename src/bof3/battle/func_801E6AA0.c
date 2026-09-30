#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6AA0
 * @behavior Copies the three completion handlers at 0x801D0F38 to the stack and
 * runs the one selected by byte +0x01 of the work object published at
 * 0x1F800044, then calls func_8014D4E0 when that object's byte +0x00 has bit 0
 * set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E6AA0(void)
{
    Battle03DispatchTable handlers;

    handlers = D_801D0F38;
    handlers.handlers[battleWork[1]]();
    if ((battleWork[0] & 1) != 0) {
        func_8014D4E0();
    }
}
