#include "bof3/bof3.h"

extern u8 D_801DB080[]; /* @source 0x801DB080 @kind data */

extern s32 func_801D2750(u8 bound);

/* @source 0x801D2870
 * @behavior Deals the nine leading card-id bytes of the work table at
 * 0x801DB080 as nine distinct ids. The first slot takes one draw; each later
 * slot keeps drawing through func_801D2750(9) - a random value in 1 through 9 -
 * until the drawn id differs from every slot already filled, so the nine bytes
 * end up a permutation of 1 through 9. Nothing else is read or written and no
 * value is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dealNineUniqueCardIds(void) {
    s32 slot;
    s32 index;
    u8 value;

    D_801DB080[0] = func_801D2750(9);
    for (slot = 1; slot < 9; slot++) {
        do {
            value = func_801D2750(9);
            index = 0;
            while (index < slot && value != D_801DB080[index]) {
                index++;
            }
        } while (index != slot);
        D_801DB080[slot] = value;
    }
}
