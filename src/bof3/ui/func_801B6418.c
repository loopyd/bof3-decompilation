#include "bof3/ui/game00_internal.h"

/*
 * @behavior Looks up the pending world id D_80143F00 in the ten packed 16-bit
 * id pairs at 0x801CD224 (key at +0, mapped id at +2, 4-byte stride) and, on the
 * first hit, stores that pair's mapped id into the context seed D_80143F10; an
 * id that matches no pair leaves the context seed unchanged.
 * @source 0x801B6418
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 22/22 instructions, 88 bytes.
 * The indexed record form (`D_801CD224[i].world_id` / `.context_seed`, 4-byte
 * stride, bound `i < 10`) reproduces the original's strength-reduced byte-offset
 * induction variable (`addiu v1,v1,4` / `slti v0,v1,0x28`) and the two symbol-
 * relative `lui $at` address materializations; the mapped id is committed to the
 * context seed and the search is left through the shared exit.
 */
void func_801B6418(void)
{
    s32 i;
    u16 world_id;

    world_id = D_80143F00;
    for (i = 0; i < 10; i++) {
        if (world_id == D_801CD224[i].world_id) {
            D_80143F10 = D_801CD224[i].context_seed;
            break;
        }
    }
}
