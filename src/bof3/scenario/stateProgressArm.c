#include "bof3/scenario/scena02_state.h"

/* @source 0x801FD3BC
 * @behavior Runs the shared front-end startup helper func_8015C088
 * (0x8015C088), sets the shared progress byte g_ScenarioProgress
 * (0x80146864) and the shared sub-state byte D_80146875 (0x80146875) to 0x14,
 * clears the shared bytes 0x80146865 and 0x80146866, and requests primary
 * scene state 2 through D_80146874 (0x80146874). The roles of the two cleared
 * bytes remain unknown. No word in this payload calls it or points at it, so
 * its parameter roles stay structural and it declares none; it exposes no
 * return value either. The 0x18-byte frame only keeps $ra across the call
 * and the delay slot is empty. The constant 0x14 is materialised once in $v0
 * and stored to both bytes, and the constant 2 is materialised in $v0 before
 * the two clearing stores, so the store order g_ScenarioProgress, D_80146875,
 * 0x80146865, 0x80146866, D_80146874 is the source statement order. The
 * nearest neighbours at 0x801FD374 and 0x801FD40C supplied the statement
 * order and declaration style; both store different constants and neither
 * sets a progress byte, so those spans are shape evidence and this body was
 * written from its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateProgressArm(void) {
  func_8015C088();
  g_ScenarioProgress = 0x14;
  D_80146875 = 0x14;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146874 = 2;
}
