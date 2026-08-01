#include "internal.h"

/* @source 0x801E4D34
 * @behavior Resets scratchpad work byte +2 after func_801E3160 succeeds, incrementing byte +1.
 */
void func_801E4D34(void)
{
    Battle03LocalWork *work;

    if (func_801E3160() != 0) {
        work = D_1F800044;
        work->unk_01++;
        D_1F800044->unk_02 = 0;
    }
}
