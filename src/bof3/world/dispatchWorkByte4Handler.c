#include "bof3/emi/dispatchers.h"

/* @source 0x801F40FC
 * @kind data
 */
extern void (*workByte4SequenceTable[])(void);

/* @source 0x801F2EE8
 * @behavior Calls the local handler selected by byte +4 of the work record
 * published at scratchpad pointer cell 0x1F800044. The unchecked two-entry
 * handler table at 0x801F40FC holds the routines at 0x801F2F2C and 0x801F2E60,
 * the second of this overlay's two work-byte-4 handler tables. Takes no
 * arguments and returns nothing; the payload's step run at 0x801F40A0
 * publishes this function as its second entry. The neighbouring dispatcher body
 * at 0x801F2D78 carries the same 68-byte body and differs only in its table
 * immediate, which a payload compare measured as 1 of 68 bytes, so that row
 * supplied the statement shape and this span's own table word is written here.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Handler, workByte4SequenceTable);
