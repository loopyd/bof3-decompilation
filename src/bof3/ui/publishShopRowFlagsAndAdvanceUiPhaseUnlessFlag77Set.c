#include "bof3/ui/shop00_internal.h"

/* @source 0x801D61C0
 * @behavior shop UI phase step at entry 0 of the phase step table D_801E5360,
 *           which func_801D6184 dispatches with the UI phase byte D_80148651 as
 *           index. It first tests the shared flag tester func_8015B5D4 over the
 *           main-RAM flag bank base D_80144F28 with the flag id 0x77: a nonzero
 *           return aborts the step by selecting entry 6 of the same table (the
 *           gated UI reset setUiFlagsGated) through the phase byte and starting
 *           the action 0xEF through func_80150224. Otherwise it clears that
 *           same flag through func_8015B580 with the bank base and 0x77, except
 *           while the shared scenario-state byte D_80146870 reads 0xE and flag 5
 *           of the shared work-context word D_8014686C passes the same tester.
 *           It then publishes the main-exe per-row flag func_8016B344(0, row)
 *           of the four rows 0 through 3 into the overlay halfword table
 *           D_801E6100, sets up every nonzero row through func_8016B968(0,
 *           row), switches the frontend-local mode through func_8014ECAC with
 *           the constant 0 and finally advances the UI phase byte by one, which
 *           hands the step to entry 1 func_801D62DC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishShopRowFlagsAndAdvanceUiPhaseUnlessFlag77Set(void) {
  u8* phaseCell;
  s16 i;
  s16 flag;

  if (func_8015B5D4((u32)&D_80144F28, 0x77) != 0) {
    D_80148651 = 6;
    func_80150224(0xEF);
    return;
  }
  if (D_80146870 != 0xE || func_8015B5D4(D_8014686C, 5) == 0) {
    func_8015B580(&D_80144F28, 0x77);
  }
  /* The four-row publish below is the same walk the sub-step handler
   * drawShopPanelStripThenPublishRowFlagsOnTimerExpiry performs, and its
   * instruction sequence is identical there. */
  for (i = 0; i < 4; i++) {
    flag = func_8016B344(0, i);
    D_801E6100[i] = flag;
    if (flag != 0) {
      func_8016B968(0, i);
    }
  }
  func_8014ECAC(0);
  /* The original materializes the phase-byte address once and reuses that base
   * for the increment load and store (lui+addiu / lbu / sb through one
   * register), which a direct D_80148651 += 1 does not: that spelling stores
   * through $at, as the exact phase step func_801D65A0 still shows. This
   * function takes the same explicitly non-volatile byte view the exact lifts
   * func_801D46E0 and drawShopVariantPanelThenRestoreUiSubstepOnTimerExpiry
   * use, with the same result. */
  phaseCell = (u8*)&D_80148651;
  *phaseCell += 1;
}
