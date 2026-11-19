#include "bof3/scenario/scena06_state.h"

/* @source 0x801FD0C4
 * @behavior Runs the shared front-end startup helper func_8015C088
 * (0x8015C088), requests secondary sub-state 0x14 in the shared sub-state byte
 * D_80146875, clears the shared progress byte g_ScenarioProgress (0x80146864)
 * and the shared bytes 0x80146865, 0x80146866 and 0x80146867, then requests
 * primary scene state 6 through D_80146874. Takes no arguments and returns
 * nothing: the record dispatcher
 * dispatchRecordCallbackScenarioScena0600_801FCDD0 (0x801FCDD0) invokes it as
 * (record, D_8014686C) and both arguments are ignored. The 0x18-byte frame only
 * keeps $ra at 0x10($sp) across the call, every byte store materialises the
 * 0x8014 page in $at on its own, the constant 0x14 is materialised in $v0
 * straight after the call to feed the sub-state store, and the constant 6
 * reaches $v0 before the four clearing stores, so the store order D_80146875,
 * g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867, D_80146874 is the
 * source statement order.
 * The address is reached only through the overlay's per-record callback table
 * at 0x801FE42C: its only image word is word 9 of that table, at 0x801FE450,
 * between the sub-state 0x14 / primary state 4 handler (0x801FD014, word 7) and
 * the reset-then-request-0xA handler (0x801FD11C, word 10), and no in-image jal
 * or j targets it. One payload cmp of the two 0x58-byte spans shows this body
 * differs from that exact sibling
 * setSubstate14ClearProgressAndSharedStateBytesAndRequestPrimaryState4ScenarioScena0600_801FD014
 * (0x801FD014) in a single byte, the immediate of the store that requests the
 * primary state (6 here, 4 there), so the sibling supplied the statement order,
 * operand forms and declaration style while this body writes its own constants
 * read from its own bytes.
 * The address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate14ClearProgressAndSharedStateBytesAndRequestPrimaryState6ScenarioScena0600_801FD0C4(void) {
  SCENA06_SET_SUBSTATE_RESET_PROGRESS_AND_REQUEST(0x14, 6);
}
