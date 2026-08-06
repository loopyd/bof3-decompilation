#include "internal.h"

/* @source 0x801E4D8C
 * @behavior Dispatches a handler selected by scratchpad work byte +0x02, then calls battle03_reset_enemy_scratch_when_bit4.
 */
void battle03_dispatch_byte2_then_reset2(void)
{
    D_801EB46C[SPAD_PTR_SLOT(u8, 0x44)[2]]();
    battle03_reset_enemy_scratch_when_bit4();
}
