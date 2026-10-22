#include "bof3/battle/battle15_internal.h"

/* @source 0x800A8360
 * @behavior Initializes the battle selection state block: the leading byte,
 *   the 0x71/0x72/0x73 and 0x79/0x7A/0x7B/0x7C/0x7D bytes, the 0x74/0x76/0x80
 *   halfwords, and the trailing 0x6C/0xDD/0xDE bytes, in the original store
 *   order.
 * @status partial
 * @match 81.58
 * @residual 31/38 instructions at the original 152-byte size. The original
 *   holds &D_80148570 in $v1 for stores at offsets 0 and 0x6C and keeps 8 in
 *   $a0. This build folds those accesses into absolute stores, changing the
 *   base and constant register allocation from the first instruction.
 */
void func_800A8360(void)
{
    BattleSelectionState *state;
    u8 one;
    u8 eight;

    state = &D_80148570;
    one = 1;
    eight = 8;
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
