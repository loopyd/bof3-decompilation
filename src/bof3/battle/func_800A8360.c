#include "bof3/battle/battle15_internal.h"

/* @source 0x800A8360
 * @behavior initializes battle selection state fields.
 * @status matching
 */
void func_800A8360(void)
{
    BattleSelectionState *state;
    u8 one;
    u8 eight;

    state = &D_80148570;
    one = 1;
    eight = 8;
    barrier();
    state->first = one;
    D_80148572 = one;
    D_80148573 = 2;
    D_8014857D = 0xFF;
    D_80148574 = -0xAA;
    D_80148571 = eight;
    D_8014857A = 0;
    D_8014857B = 0;
    D_8014857C = 0;
    D_80148579 = 0;
    D_80148580 = 0;
    D_80148576 = 0x3F;
    state->field_6C = 0;
    D_801485DD = eight;
    D_801485DE = 0;
}
