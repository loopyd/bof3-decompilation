#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801D0A54: thirty code pointers, from
 * 0x801CE448 through 0x801CFB50, with the shared main-executable handler
 * 0x801B2D54 at entries 2 and 8. The Splat `textbin` row of the same name
 * places the words, so the table is only referenced here. This dispatcher
 * indexes it from 0x801D0A60, so its base is entry 3 and entry 0 is 0x801CE770,
 * that is the same run reached three entries further in than the overlay's
 * other byte-2 dispatcher at 0x801CE404. */
extern void (*D_801D0A60[])(void); /* @source 0x801D0A60 @kind data */

/* @source 0x801CE72C
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x801D0A60 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call, and the slot after the index byte and the slot
 * after the table load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table0a60PlcharPlp02600_801CE72C, D_801D0A60);
