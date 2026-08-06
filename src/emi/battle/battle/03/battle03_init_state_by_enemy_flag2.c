#include "internal.h"

/* @source 0x801E35D8
 * @behavior conditionally initializes battle state, then advances scratchpad work byte 1.
 */
void battle03_init_state_by_enemy_flag2(void)
{
    u32            flags;
    volatile void* enemy_work;

    enemy_work = PSX_REF(volatile void*, 0x801EB4E8u);
    flags = FIELD_REF(u32, enemy_work, 0x100);
    if (flags & 2) {
        func_801E2314(8);
    } else {
        func_801E2314(0);
    }
    battle03_enemy_ready_or_helper1();
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
