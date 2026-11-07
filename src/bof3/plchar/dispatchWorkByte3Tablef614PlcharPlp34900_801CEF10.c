#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF5EC: sixteen code pointers, 0x801CF5EC
 * through 0x801CF628, holding this payload's functions plus the shared
 * main-executable handler 0x801B2D54 at entries 2 and 15. The Splat `textbin`
 * row at 0x801CF5EC places the words, so the run is only referenced here. This
 * dispatcher indexes it from 0x801CF614, its tenth entry, so the base is
 * 0x801CF614 and the six reachable entries are 0x801CF614 through 0x801CF628.
 * That base is the same run reached three entries further in than the overlay's
 * other byte-3 dispatcher at 0x801CE654, which bases at 0x801CF600. */
extern void (*D_801CF614[])(void); /* @source 0x801CF614 @kind data */

/* @source 0x801CEF10
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record's unsigned byte at offset 3, scales it by four and
 *           calls the matching entry of this overlay's handler run based at
 *           0x801CF614 with no arguments, discarding any result. The original
 *           does not bound the index before the scaled table load. It reads no
 *           other state and writes none; the pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slot after the index byte and the slot after
 *           the table load stay nops. Its only in-image reference is the word
 *           0x801CEF10 at 0x801CF5FC, entry 4 of the sixteen-entry code-pointer
 *           run at 0x801CF5EC, so the overlay loader that steps that run is what
 *           calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchWorkByte3Tablef614PlcharPlp34900_801CEF10, D_801CF614);
