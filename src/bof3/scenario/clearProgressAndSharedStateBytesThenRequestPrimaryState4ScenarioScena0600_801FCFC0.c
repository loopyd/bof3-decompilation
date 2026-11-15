#include "bof3/scenario/scena06_state.h"

/* @source 0x801FCFC0
 * @behavior Runs helper func_8015C088, clears the progress byte at 0x80146864
 * and shared bytes 0x80146865..0x80146867, clears the secondary state byte
 * D_80146875, then requests primary state 4 through D_80146874. This is word
 * 6 of the record callback table at 0x801FE42C; it ignores the record pointer
 * and shared-word arguments supplied by the dispatcher and returns no value.
 * The six byte stores follow the helper in that order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState4ScenarioScena0600_801FCFC0(void) {
  SCENA06_RESET_PROGRESS_AND_REQUEST(4);
}
