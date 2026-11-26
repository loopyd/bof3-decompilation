#include "bof3/emi/dispatchers.h"

/* @source 0x801F49A0
 * @kind data
 */
extern void (*byte4HandlerTableB[])(void);

/* @source 0x801F2E58
 * @behavior Calls the local handler selected by byte +4 of the work record
 * published at scratchpad pointer cell 0x1F800044. The unchecked handler table
 * at 0x801F49A0 contains the routines at 0x801F2E9C and 0x801F2FD4, the second
 * of the overlay's two work-byte-4 handler tables. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, callWorkByte4Table49, byte4HandlerTableB);
