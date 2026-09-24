#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801F6D04
 * @behavior Resets this overlay's per-frame work state: it first re-arms the
 * @status exact
 * @match 100.00
 * @residual none
 * three-entry overlay effect group through func_80166E88(0, 3, 4, 0), then
 * clears the shared live-local-work-record count byte D_80146254 to zero, and
 * finally runs the per-id teardown func_801C187C for ids 0, 3 and 4 in that
 * order. It is entry 5 of this overlay's four-byte handler table at 0x801F6D6C
 * (indexed by the signed progress byte D_80146872 in
 * dispatchProgressHandler_scena18), takes no arguments, returns nothing and
 * reads no other memory.
 */
void clearWorkRecordsAndResetEffectSlots_scena18(void) {
  func_80166E88(0, 3, 4, 0);
  D_80146254 = 0;
  func_801C187C(0);
  func_801C187C(3);
  func_801C187C(4);
}
