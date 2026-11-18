#include "bof3/emi/dispatchers.h"

/* Handler pointer run of this overlay. The Splat textbin row D_8011818C places
 * thirteen in-image code pointers, 0x8011818C through 0x801181BC. This
 * dispatcher indexes it from 0x8011819C, so the base is entry 4 and the nine
 * reachable entries begin with
 * raiseModeFlagBit40ReseedWorkRecordsArmCountdownByte9To180ThenAdvanceStateWorld01Area04913_8011765C,
 * followed by claimSeedWorkRecordPublishWord1cEveryFourthFrameThenCountdownScratchByte9RearmTo40PublishWord0cAndAdvanceStateWorld01Area04913_801176B8, func_80117768,
 * func_80117818, func_80117A74, func_80117ACC, func_80117C38, func_80117C58 and
 * func_80117DF0, the last word of the run. The run stays one row, so this
 * declaration binds the interior word through the generated
 * undefined_syms_auto.txt row and owes no target-map row. */
extern void (*D_8011819C[])(void); /* @source 0x8011819C @kind data */

/* Record pass of this overlay: sets the draw state up and walks the sixteen
 * records of the 0x800E4800 work table, repointing the record cell world01Area04913WorkRecordCursor
 * once per record and running the renderer func_801170D8 on each claimed one. */
extern void func_80117900(void); /* @source 0x80117900 */

/* @source 0x80117610
 * @behavior Step dispatch of this overlay on work-record byte 1: loads
 * the work-record pointer published at the scratchpad pointer cell 0x1F800044,
 * takes that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of the handler run based at 0x8011819C, entry 4 of the
 * thirteen-pointer run the Splat textbin row D_8011818C places, with no
 * arguments. The original does not bound the index before the scaled load. Once
 * the handler returns it runs the overlay's record pass func_80117900. It reads
 * no other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the two calls, and the slot
 * after the index byte and the slot after the table load stay nops. No call or
 * pointer word in the payload names this function, so it is an overlay entry
 * rather than an in-payload callee. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER_THEN_RESET(
    EMI_DISPATCH_WORK_BYTE_1,
    dispatchWorkByte1Table819cThenRunRecordPassWorld01Area04913_80117610,
    D_8011819C, func_80117900);
