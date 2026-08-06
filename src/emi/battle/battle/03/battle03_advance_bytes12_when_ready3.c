#include "internal.h"

/* @source 0x801E4D34
 * @behavior Resets scratchpad work byte +2 after battle03_enemy_ready_or_helper2 succeeds, incrementing byte +1.
 */
void battle03_advance_bytes12_when_ready3(void)
{
    Battle03LocalWork *work;

    if (battle03_enemy_ready_or_helper2() != 0) {
        work = D_1F800044;
        work->unk_01++;
        D_1F800044->unk_02 = 0;
    }
}
