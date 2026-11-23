#include "bof3/emi/dispatchers.h"

/* @source 0x801F40F0
 * @kind data
 */
extern void (*workByte4HandlersWorld00Area01513[])(void);

/* @source 0x801F2D78
 * @behavior Calls the local handler selected by byte +4 of the work record
 * published at scratchpad pointer cell 0x1F800044. The unchecked two-entry
 * handler table at 0x801F40F0 holds the routines at 0x801F2DBC and 0x801F2E60.
 * Takes no arguments and returns nothing; the payload's step run at 0x801F40A0
 * publishes this function as one of its entries. The neighbouring asm row at
 * 0x801F2EE8 carried the same 68-byte body and differed only in its table
 * immediate, which a payload compare measured as 1 of 68 bytes, so that row was
 * shape evidence and not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, callWorkByte4Table0, workByte4HandlersWorld00Area01513);
