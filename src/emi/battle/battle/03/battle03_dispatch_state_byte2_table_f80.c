#include "internal.h"

/* @source 0x801E9074
 * @behavior copies the eight-handler local dispatch table to the stack and
 * invokes the entry selected by battle state byte +0x02.
 */
void battle03_dispatch_state_byte2_table_f80(void)
{
    Battle03EightDispatchTable handlers;

    handlers = D_801D0F80;
    handlers.handlers[((u8*)D_80148648)[2]]();
}
