#include "bof3/emi/dispatchers.h"

/* This overlay's handler table: sixteen code pointers, 0x801CF5EC through
 * 0x801CF628, holding this payload's functions plus the shared main-executable
 * handler 0x801B2D54 at entries 2 and 15. The Splat `textbin` row at 0x801CF5EC
 * places the words, so the table is only referenced here. This dispatcher
 * indexes it from 0x801CF600, so the base is entry 5 and the reachable entries
 * are 0x801CF600 through 0x801CF628. */
extern void (*D_801CF600[])(void); /* @source 0x801CF600 @kind data */

/* @source 0x801CE654
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 3, scales it by four and calls
 *           the matching entry of this overlay's handler table based at
 *           0x801CF600 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled load. It reads no other
 *           state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CE654 at 0x801CF5F8, entry 3 of the sixteen-entry code-pointer
 *           run at 0x801CF5EC, which the byte-2 dispatcher based at that entry
 *           reaches as index 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Tablef600PlcharPlp34900_801CE654, D_801CF600);
