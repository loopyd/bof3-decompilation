#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C7BEC) indexed by the panel
 * task byte (task+0x02) through a tail-style jalr call.
 * @source 0x80199558
 */
void func_80199558(void)
{
    D_801C7BEC[g_PanelTaskRoot->unk_00[2]]();
}
