#include "bof3/emi/dispatchers.h"

extern void (*dispatchHandlerTable801EFBB0[])(void); /* @source 0x801EFBB0 @kind data */
extern void (*D_801EFBB4[])(void); /* @source 0x801EFBB4 @kind data */

/* @source 0x801EEFF8
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local thirteen-entry code-pointer table at 0x801EFBB0 (the entry is
 * read back with lw from that table), and the selected entry is invoked with no
 * arguments. It reads no state other than that byte and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call. The bytes are
 * identical to the sibling dispatchers of this overlay family that index a
 * different table (emi/bmagic/magic111/03@0x801EEFF8 and
 * emi/bmagic/magic131/03@0x801EF668), apart from the table relocation itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerBmagicMagic01200_801EEFF8, dispatchHandlerTable801EFBB0);

/* @source 0x801EF03C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 2 is scaled by four and selects
 * an entry of the overlay-local thirteen-entry code-pointer table at
 * 0x801EFBB0, and the selected entry is invoked with no arguments. The index is
 * offset by one entry because entry 0 of that table is this dispatcher itself:
 * work byte 2 value 0 invokes 0x801EF080, the overlay's first work-byte-2
 * state handler. It reads no other state and writes none; the 0x18-byte frame
 * only keeps $ra across the indirect call, and the pointer-cell load precedes
 * frame creation. The selection sequence is byte-identical to the work-byte-1
 * dispatcher at 0x801EEFF8 of this overlay apart from the work-object byte
 * offset (2 instead of 1) and the table base (+4, the entry offset).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2HandlerBmagicMagic01200_801EF03C, D_801EFBB4);
