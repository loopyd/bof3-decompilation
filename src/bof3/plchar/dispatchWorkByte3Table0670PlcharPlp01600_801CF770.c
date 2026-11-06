#include "bof3/emi/dispatchers.h"

/* This overlay's word run at 0x801D0630: thirty code pointers, entry 0 =
 * 0x801CE448 through entry 29 = 0x801CF72C, holding this payload's functions plus
 * the shared main-executable handler 0x801B2D54 at entries 2 and 8. The Splat
 * `textbin` row of the same name places the words, so the run is only referenced
 * here. This dispatcher indexes it from 0x801D0670, so its base is entry 16
 * (0x801CF7B4) and the entries it can reach are entries 16 through 29. */
extern void (*D_801D0670[])(void); /* @source 0x801D0670 @kind data */

/* @source 0x801CF770
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801D0670 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops. Its only in-image reference is the word
 * 0x801CF770 at 0x801D0668, entry 14 of the thirty-word code-pointer run at
 * 0x801D0630, so the byte-2 dispatcher at 0x801CF72C, whose base is that entry,
 * is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Table0670PlcharPlp01600_801CF770, D_801D0670);
