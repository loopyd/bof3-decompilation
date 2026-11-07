#include "bof3/emi/dispatchers.h"

/* This overlay's handler table: the five-entry table of the sibling dispatcher
 * at 0x800C20F0 starts at 0x800C3294 and its last two entries are 0x800C3038 and
 * 0x800C3088, the words at 0x800C32A0 and 0x800C32A4; the word after them at
 * 0x800C32A8 begins the "turns.left" string. The reviewed Splat layout places
 * those words in the trailing `textbin` row D_800C3174, so the run is only
 * referenced here. This dispatcher indexes that table at its third entry, so the
 * handlers it can reach are 0x800C3038 and 0x800C3088. */
extern void (*D_800C32A0[])(void); /* @source 0x800C32A0 @kind data */

/* @source 0x800C2FF4
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte at offset 1, scales it by four and calls
 *           the matching entry of this overlay's handler run based at
 *           0x800C32A0 with no arguments. The original does not bound the index
 *           before the scaled load. It reads no other state and writes none; the
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slot after the
 *           index byte and the slot after the table load stay nops. No in-image
 *           reference to this address exists in the payload: no word equals
 *           0x800C2FF4 and no lui/addiu pair forms it, so neither a `jal` nor a
 *           table entry reaches it and it can only be entered from outside the
 *           overlay. Same body as the sibling dispatchers of this overlay, which
 *           read work-record byte 1 or 2 or index another word of the same run;
 *           only the record byte and the table address in the load word differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table32a0BossBoss02513_800C2FF4, D_800C32A0);
