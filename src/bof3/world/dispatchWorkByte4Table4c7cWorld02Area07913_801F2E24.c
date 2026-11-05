#include "bof3/emi/dispatchers.h"

/* @source 0x801F4C7C
 * @kind data
 */
extern void (*workByte4HandlersWorld02Area07913[])(void);

/* @source 0x801F2E24
 * @behavior Calls the local handler selected by byte +4 of the work record
 * published at scratchpad pointer cell 0x1F800044. The unchecked handler table
 * at 0x801F4C7C contains the routines at 0x801F2E68 and 0x801F2EA8. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table4c7cWorld02Area07913_801F2E24, workByte4HandlersWorld02Area07913);
