#include "bof3/scenario/scena06_state.h"

/* @source 0x801FD11C
 * @behavior Runs helper func_8015C088, clears the progress byte at 0x80146864
 * and shared bytes 0x80146865..0x80146867, clears the secondary sub-state byte
 * D_80146875, then requests primary state 0xA through D_80146874. The constant
 * is materialised once in $v0 and reused by that store, the 0x18-byte frame
 * only keeps $ra across the call, and the two arguments the record dispatcher
 * passes (the record pointer and the shared D_8014686C word) are ignored, so
 * the function takes none and returns nothing. The address is reached only
 * through the overlay's per-record callback table at 0x801FE42C: its only image
 * word is word 10 of that table, at 0x801FE454, and no in-image jal targets it.
 * This 84-byte body differs from 0x801FCE18, 0x801FCE6C and 0x801FCFC0 only
 * in the immediate that sets the requested primary state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateRequestPrimary10(void) {
  SCENA06_RESET_PROGRESS_AND_REQUEST(0xA);
}
