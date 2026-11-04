#include "bof3/emi/dispatchers.h"

extern void (*D_801F576C[])(void); /* @source 0x801F576C @kind data */
extern void (*D_801F57D0[])(void); /* @source 0x801F57D0 @kind data */

/* @source 0x801F3B5C
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table
 *           based at 0x801F576C. The selected handler's $v0 is left untouched
 *           by the epilogue, so this shim's result is whatever that handler
 *           returns. Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte2Table6284World02Area07714_801F4CDC, and the
 *           twin of this target's 0x801F49D4, which dispatches the same work
 *           byte +0x02 through the table based at 0x801F57D0.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table576cWorld02Area08213_801F3B5C, D_801F576C);

/* @source 0x801F49D4
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table
 *           based at 0x801F57D0. The selected handler's $v0 is left untouched
 *           by the epilogue, so this shim's result is whatever that handler
 *           returns. Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte2Table6284World02Area07714_801F4CDC, and the
 *           twin of this target's 0x801F3B5C, which dispatches the same work
 *           byte +0x02 through the table based at 0x801F576C.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table57d0World02Area08213_801F49D4, D_801F57D0);
