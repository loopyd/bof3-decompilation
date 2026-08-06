#include "internal.h"

/* @source 0x801E5C1C
 * @behavior copies the eight-handler local dispatch table to the stack and
 * invokes the entry selected by queued-slot byte +0x05.
 */
void battle03_dispatch_slot_byte5_table_ed4(void)
{
    Battle03EightDispatchTable handlers;

    handlers = D_801D0ED4;
    handlers.handlers[D_801EC2E0->unk_05]();
}
