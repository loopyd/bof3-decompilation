#include "bof3/emi/dispatchers.h"

/* This overlay's step-handler run: the target's Splat `textbin` row angleStepHandlerTable
 * labels eight in-image code pointers, entry 0 at 0x801D0C48 through entry 7 at
 * 0x801D11B4, and that data object defines the base this body indexes. */
extern void (*angleStepHandlerTable[])(void); /* @source 0x801D1F88 @kind table */

/* @source 0x801D0C04
 * @behavior Overlay step dispatch on work-record byte 1 of this payload, and the
 * first word of its code: it loads the work-object pointer published at the
 * scratchpad pointer cell 0x1F800044 before it creates its frame, takes that
 * object's unsigned byte at offset +0x1, scales it by four and calls that entry of
 * the eight-word handler run based at 0x801D1F88 with no arguments, discarding any
 * result. It reads no other state and writes none, and the handlers it reaches
 * advance that same +0x1 byte themselves, so one byte both selects the step and
 * records progress through the run. The original does not bound the index before
 * the scaled table load, so a work byte above 7 reads past the run into the 16
 * zero bytes of the trailing row angleStepPadTable. Its 0x18-byte frame only keeps $ra at
 * 0x10 across the indirect call, and the index load's delay slot and the table
 * load's delay slot both stay nops. No `jal` and no word of this payload holds
 * 0x801D0C04, so the overlay loader outside the payload enters it there. Its 68
 * bytes are the 17-instruction dispatcher shape that the sibling dispatcher bodies
 * dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4 and
 * dispatchWorkByte1HandlerScenarioSce15ef200_801D10A4 in the sce15ef2 overlays
 * record, and this body differs from them only in the table address immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callAngleBandHandler, callAngleBandHandler, angleStepHandlerTable);
