#include "bof3/emi/dispatchers.h"

/* @source 0x801F4988
 * @kind data
 */
extern void (*workByte4HandlersWorld02Area09913[])(void);

/* @source 0x801F2D68
 * @behavior Calls the local handler selected by byte +4 of the work record
 * published at scratchpad pointer cell 0x1F800044. The unchecked handler table
 * at 0x801F4988 contains the routines at 0x801F2DAC and 0x801F2DEC. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table4988World02Area09913_801F2D68, workByte4HandlersWorld02Area09913);
