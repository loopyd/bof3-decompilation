#include "bof3/bof3.h"

/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80144F28 @kind unknown */
extern u8 D_80144F28;

void func_8015C088(void);
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801FDEA4
 * @behavior Overlay coordinate-lookup handler reached only indirectly: no
 * in-image jal targets the address, and its only image word is the pointer at
 * 0x801FE498, word 3 of the 22-word code-pointer run 0x801FE48C..0x801FE4E4
 * that dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14 (0x801FDD14)
 * indexes with the answer of func_801A7AD8 and calls with jalr, so this body
 * takes no arguments and answers 1. It first runs the shared flag tester
 * func_8015B5D4 (0x8015B5D4) on the shared scenario flag word D_8014686C
 * (0x8014686C) for flag 0x27; a zero answer runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), clears the shared progress byte
 * g_ScenarioProgress (0x80146864) and the shared bytes D_80146865..D_80146867
 * (0x80146865..0x80146867) in that order, and clears the shared sub-state byte
 * D_80146875 (0x80146875). A non-zero answer tests the main-RAM flag bank at
 * D_80144F28 (0x80144F28), passed by address as the tester base, for flags 5
 * and 6: either flag answering zero takes the same startup-and-clear run,
 * which is why those six byte stores appear twice in the span. Otherwise the
 * same tester is asked for flag 0x2C on D_8014686C: a zero answer runs the
 * startup helper, arms sub-state 5 in D_80146875 and clears the four progress
 * bytes, while a non-zero answer runs it and arms sub-state 0x1E there.
 * Every arm then requests primary scene state 0x0F in the shared state byte
 * D_80146874 (0x80146874) and answers 1, and the compiler cross-jumped those
 * tail stores into the single site at 0x801FDFE0, with the 0x0F filled into
 * each arm jump delay slot and 1 answering from $v0. The first wait id is
 * materialised above the $ra save, and the 0x18-byte frame keeps $ra plus $s0,
 * which holds the D_80144F28 base across the two bank probes. The neighbouring
 * table words
 * waitFlag27ThenClearProgressAndSharedStateBytesAndRequestPrimaryState0DScenarioScena0600_801FDE34
 * (0x801FDE34, word 2) and the flag-0x05 record callback at 0x801FCEC0 kept the
 * same wait-then-clear shape for primary states 0x0D and 0x02 when this body
 * was written, so they were shape evidence and not clone proof.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlag27ElseMainRamFlags5Or6ElseFlag2CThenClearProgressAndSharedStateBytesAndRequestPrimaryState0FSubstate1EOr5ScenarioScena0600_801FDEA4(void) {
  if (func_8015B5D4(D_8014686C, 0x27) == 0) {
    func_8015C088();
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0x0F;
    return 1;
  }
  if (func_8015B5D4((u32)&D_80144F28, 5) == 0 ||
      func_8015B5D4((u32)&D_80144F28, 6) == 0) {
    func_8015C088();
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0x0F;
    return 1;
  }
  if (func_8015B5D4(D_8014686C, 0x2C) == 0) {
    func_8015C088();
    D_80146875 = 5;
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146874 = 0x0F;
    return 1;
  }
  else {
    func_8015C088();
    D_80146875 = 0x1E;
    D_80146874 = 0x0F;
    return 1;
  }
}
