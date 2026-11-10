#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u8 *D_1F800044;

/* @source 0x801F04C8 @kind data */
extern void (*D_801F04C8[])(void);

/* @source 0x801EF688 @kind function */
extern void func_801EF688(void);
/* @source 0x801EF79C @kind function */
extern void func_801EF79C(void);

/* @source 0x801EF314
 * @behavior Calls the handler indexed by unsigned work byte 2 through the
 * table at 0x801F04C8, without a bounds check. Reloads the work pointer from
 * scratchpad cell 0x1F800044 after the handler returns. When work byte 0 is
 * nonzero, invokes 0x801EF688, 0x801EF79C, and PopMatrix in that order.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvoke0688And079CAndPopWhenWorkByte0NonzeroBmagicMagic06703_801EF314(void) {
    u8 *step_record;
    u8 *active_record;

    step_record = D_1F800044;
    D_801F04C8[step_record[2]]();
    active_record = D_1F800044;
    if (active_record[0] != 0) {
        func_801EF688();
        func_801EF79C();
        PopMatrix();
    }
}
