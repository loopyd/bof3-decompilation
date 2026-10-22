#include "bof3/battle/battle03_internal.h"

/* @source 0x801E7818
 * @behavior Copies the five saved-preview/result handlers bound at 0x801D0F6C
 * to the stack and runs the one selected by byte +0x01 of the work object
 * published at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E7818(void)
{
    Battle03FiveDispatchTable handlers;

    handlers = D_801D0F6C;
    handlers.handlers[battleWork[1]]();
}
