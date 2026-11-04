#include "bof3/emi/dispatchers.h"

extern void (*D_800C2660[])(void); /* @source 0x800C2660 @kind data */
extern void (*D_800C26CC[])(void); /* @source 0x800C26CC @kind data */
extern void (*D_800C2708[])(void); /* @source 0x800C2708 @kind data */
extern void (*D_800C2690[])(void); /* @source 0x800C2690 @kind data */


/* @source 0x800C1804
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x01 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C2660. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Same shape as the already-exact sibling
 * dispatchStateByte1Table6268World02Area07714_801F3E04. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2660BossBoss00816_800C1804, D_800C2660);

/* @source 0x800C1CD8
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x01 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C26CC. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Same shape as the already-exact sibling
 * dispatchStateByte1Table2660BossBoss00816_800C1804, which reads the same
 * dispatch byte through the table based at 0x800C2660. Takes no arguments and
 * forwards none.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table26ccBossBoss00816_800C1CD8, D_800C26CC);

/* @source 0x800C1DC4
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x01 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C2708. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Same shape as the already-exact sibling
 * dispatchStateByte1Table2660BossBoss00816_800C1804, which reads the same
 * dispatch byte through the table based at 0x800C2660. Takes no arguments and
 * forwards none.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2708BossBoss00816_800C1DC4, D_800C2708);

/* @source 0x800C18A0
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x02 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C2690. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Takes no arguments and no arguments are forwarded.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table2690BossBoss00816_800C18A0, D_800C2690);
