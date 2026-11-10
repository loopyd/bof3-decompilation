#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
/* @source 0x80155218 */
extern void func_80155218(s32 arg0, s32 arg1);
/* @source 0x801F3950 */
extern void func_801F3950(s32 arg0, s32 arg1);

/* @source 0x801F38C4
 * @behavior Subtracts 16 from the scratchpad work record's halfword at +0x3E.
 * When its signed value is nonpositive, calls 0x80155218 with the signed
 * halfwords at +0x36 and +0x3A incremented by one, then reloads the work
 * pointer and sets state byte +0x02 to 1. Reloads the pointer again and calls
 * 0x801F3950 with byte +0x08 and the signed halfword at +0x3E.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkHalfword3eWorld03Area14013_801F38C4(void) {
    u16 *work;
    u16 *tailWork;

    work = g_WorkRecordWorld03Area14013;
    work[0x1F] -= 16;
    if ((s16)work[0x1F] <= 0) {
        func_80155218((s16)(work[0x1B] + 1), (s16)(work[0x1D] + 1));
        ((u8 *)g_WorkRecordWorld03Area14013)[2] = 1;
    }
    tailWork = g_WorkRecordWorld03Area14013;
    func_801F3950(((u8 *)tailWork)[8], (s16)tailWork[0x1F]);
}
