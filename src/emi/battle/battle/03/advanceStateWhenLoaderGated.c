#include "internal.h"

/* @behavior advances the battle state dispatcher after the EXE-side loader
 * is ready and its gate is enabled.
 * @source 0x801D6D90
 */
void advanceStateWhenLoaderGated(void)
{
    if (func_80162D00() != 0 && D_80145AA8 != 0) {
        volatile u8* state = &BATTLE_GLOBAL_BYTE_62E2;

        *state += 1;
    }
}
