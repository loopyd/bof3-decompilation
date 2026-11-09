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

void func_8015C088(void);
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801FDE34
 * @behavior Overlay coordinate-lookup handler reached only indirectly: no
 * in-image jal targets the address, and its only image word is the pointer at
 * 0x801FE494, word 2 of the 22-word code-pointer run 0x801FE48C..0x801FE4E4
 * (scenarioScena0600CoordinateLookupHandlers). The run is dispatched by
 * dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14 (0x801FDD14), which
 * forwards two unsigned halfword coordinates and the four five-byte lookup
 * records at 0x801FE478 to func_801A7AD8 and calls entry index of the answer,
 * so only entries 0 through 3 are reachable and this body takes no arguments
 * and answers as a signed byte. It first takes the two-stage wait
 * func_8015B5D4 (0x8015B5D4) on the shared scenario flag word D_8014686C
 * (0x8014686C, read with lui/lw into $a0 above the frame's $ra save) for flag
 * 0x27, which is materialised in the jal delay slot, and answers 1 immediately
 * when that wait reads non-zero. A zero wait result runs the shared front-end
 * startup helper func_8015C088 (0x8015C088), clears the shared progress byte
 * g_ScenarioProgress (0x80146864) and the shared bytes D_80146865..D_80146867
 * (0x80146865..0x80146867) in that order, clears the shared sub-state byte
 * D_80146875 (0x80146875), then requests primary scene state 0x0D in the shared
 * state byte D_80146874 (0x80146874) and answers 1 as well. Its 0x18-byte frame
 * only keeps $ra across the two calls, and each byte store materialises the
 * 0x8014 page in $at on its own. The following table word func_801FDEA4
 * (0x801FDEA4) keeps the same flag-0x27 probe and clear run but arms sub-state
 * 0x0F and adds the D_80144F28 flag-0x05/0x06 legs, and the per-record callback
 * word waitFlag5ThenSetSubstate0AClearProgressAndSharedStateBytesAndRequestPrimaryState2ScenarioScena0600_801FCEC0
 * (0x801FCEC0) keeps the same wait-then-clear-then-request shape for flag 0x05
 * and primary state 0x02, so those neighbours are shape evidence and not clone
 * proof; no other payload in the tree carries these 0x70 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlag27ThenClearProgressAndSharedStateBytesAndRequestPrimaryState0DScenarioScena0600_801FDE34(void) {
  if (func_8015B5D4(D_8014686C, 0x27) == 0) {
    func_8015C088();
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0x0D;
    return 1;
  }
  return 1;
}
