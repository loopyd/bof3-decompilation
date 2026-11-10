#include "bof3/bof3.h"

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
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
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

void func_801BE1B0(u32 arg0);
void func_80161D20(s32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_80161D74(s32 arg0);
void func_8015C058(void);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FA760
 * @behavior Primary state 10 of this overlay, reached as word 10 of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through seven states, each
 *           gated on the shared front-end progress byte g_ScenarioProgress.
 *           State 0, unless the shared front-end phase byte D_80143BB0 reads 2,
 *           raises the progress byte to 1, runs the GAME.EMI helper
 *           func_801BE1B0 with mode 7 and arms secondary state 1; state 1, at
 *           progress 8, opens the shared front-end transition for selector
 *           0x42 through func_80161D20, arms effect group 2 with flag 1 through
 *           func_801636A0 and advances to state 2; state 2, once the main-EXE
 *           front-end transition predicate func_80163EA0 reports a nonzero low
 *           byte, hands selector 0x42 to func_80161D74, raises the progress
 *           byte to 9 and advances to state 3; state 3, at progress 0x0A, runs
 *           the shared release helper func_8015C058 and clears the secondary
 *           and primary state bytes. There is no state 4 case, so its handler
 *           word falls to the epilogue. State 5 runs func_801BE1B0 with mode 8,
 *           sets flag bit 0x24 of the shared scenario flag word D_8014686C
 *           through func_8015B580 and advances to state 6; state 6, at progress
 *           0x17, runs the release helper, clears the shared bytes
 *           0x80146865..0x80146867 and then the secondary and primary state
 *           bytes. Every other secondary state value falls to the epilogue. The
 *           0x18-byte frame keeps $ra and $s0 across the calls, the
 *           state-byte load and its bound test are hoisted above the frame
 *           setup, and the seven-word case table sits at 0x801F7024.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState10ScenarioScena0600_801FA760(
    void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 1u;
      func_801BE1B0(7u);
      D_80146875 = 1u;
      break;

    case 1:
      if (g_ScenarioProgress == 8u) {
        func_80161D20(0x42);
        func_801636A0(2u, 1u);
        D_80146875 = 2u;
      }
      break;

    case 2:
      if ((u8)func_80163EA0() != 0u) {
        func_80161D74(0x42);
        g_ScenarioProgress = 9u;
        D_80146875 = 3u;
      }
      break;

    case 3:
      if (g_ScenarioProgress == 0xAu) {
        func_8015C058();
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;

    case 5:
      func_801BE1B0(8u);
      func_8015B580(D_8014686C, 0x24);
      D_80146875 = 6u;
      break;

    case 6:
      if (g_ScenarioProgress == 0x17u) {
        func_8015C058();
        D_80146865 = 0u;
        D_80146866 = 0u;
        D_80146867 = 0u;
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;
  }
}
