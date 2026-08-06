#include "internal.h"

/* @source 0x801E4F64
 * @behavior reads the non-volatile scratchpad pointer cell at 0x1F800044,
 * selects its byte at offset 0x02, and calls that entry of D_801EB478.
 */
void battle03_dispatch_byte2_table_b478(void)
{
    D_801EB478[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
