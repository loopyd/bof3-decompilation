#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801F3898: three code pointers - 0x801F32C8,
 * 0x801F3354 and this target's own
 * clearWorldFlagAndWorkByte4World01Area05613_801F3440 at 0x801F3440. The Splat
 * `textbin` row D_801F34DC places the words, so the table is only referenced
 * here. */
extern void (*workByte4HandlersWorld01Area05613[])(void); /* @source 0x801F3898 @kind data */

/* @source 0x801F3284
 * @behavior Overlay step dispatch: reads the work-record pointer published at
 * the scratchpad pointer cell 0x1F800044 once, takes its byte at offset 4,
 * scales that byte by four and calls the matching entry of this overlay's
 * three-entry handler table at 0x801F3898 with no arguments, discarding any
 * result. The chosen entry is read from the table itself, not from a frame
 * copy. It reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra at 0x10 across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4, dispatchWorkByte4Table3898World01Area05613_801F3284, workByte4HandlersWorld01Area05613);
