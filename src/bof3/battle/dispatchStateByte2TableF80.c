#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9074
 * @behavior copies the eight-handler local dispatch table to the stack and
 * invokes the entry selected by battle state byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateByte2TableF80(void)
{
    Battle03EightDispatchTable handlers;

    handlers = D_801D0F80;
    handlers.handlers[((u8*)D_80148648)[2]]();
}
