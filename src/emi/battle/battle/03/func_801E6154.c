#include "internal.h"

// @source 0x801E6154
// @behavior Advances the queued-slot position or marks its state when the target is reached.
void func_801E6154(void) {
    Battle03QueuedSlot *work;
    Battle03QueuedSlot *new_var;

    work = D_801EC2E0;
    if ((FIELD_REF(s16, work, 0x3a) >= FIELD_REF(s32, work, 0x1c)) &&
        (FIELD_REF(s32, work, 0x18) != FIELD_REF(s16, work, 0x36))) {
        work->unk_09 = 10;
        D_801EC2E0->unk_01++;
    } else {
        work = new_var = D_801EC2E0;
        FIELD_REF(s32, work, 0x34) += FIELD_REF(s32, work, 0x0c);
        FIELD_REF(s32, new_var, 0x38) += FIELD_REF(s32, new_var, 0x10);
        FIELD_REF(s32, new_var, 0x10) += 0x14000;
    }
}
