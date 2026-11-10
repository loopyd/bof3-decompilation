#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
/* @source 0x80143BB0 @kind data */
extern u8 D_80143BB0;
/* @source 0x8015DF18 */
extern void func_8015DF18(u16 cue);
/* @source 0x801F3050 */
extern void func_801F3050(void);

/* @source 0x801F2F7C
 * @behavior Adds 0x20 to work halfword +0x30 and stores the truncated value.
 * If that value is nonnegative as a signed halfword, queues cue 0x201 through
 * 0x8015DF18 when byte 0x80143BB0 is zero, reloads the scratchpad work pointer
 * and sets state byte +0x02 to 1. Calls the local drawing routine 0x801F3050
 * on every path. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementHalfword30By20AndSetWorkByte2To1WhenNonNegativeThenDrawWorld03Area14013_801F2F7C(void) {
    u16 *work;
    u16 half;

    work = g_WorkRecordWorld03Area14013;
    half = work[0x18] + 0x20;
    work[0x18] = half;
    if ((s16)half >= 0) {
        if (D_80143BB0 == 0) {
            func_8015DF18(0x201);
        }
        ((u8 *)g_WorkRecordWorld03Area14013)[2] = 1;
    }
    func_801F3050();
}
