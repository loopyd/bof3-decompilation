#include "bof3/emi/dispatchers.h"

/* @source 0x801F63C4
 * @kind data
 */
extern void (*workByte4HandlersWorld02Area10814[])(void);

/* @source 0x801F4B14
 * @behavior Calls the local handler selected by byte +4 of the work record
 * published at scratchpad pointer cell 0x1F800044. The unchecked handler table
 * at 0x801F63C4 contains the routines at 0x801F4B58 and 0x801F4C28. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table63c4World02Area10814_801F4B14, workByte4HandlersWorld02Area10814);
