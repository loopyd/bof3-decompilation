#include "bof3/bof3.h"

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146876 @kind unknown */
extern u16 D_80146876;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* @source 0x80149332 @kind unknown */
extern u8 D_80149332;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8019FA28(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);

/* @source 0x801F7A44
 * @behavior Primary state 1 of this overlay, reached as word 1 of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through an eleven-entry case
 *           table, and every value of that byte above 0x0A falls to the
 *           epilogue. State 0, unless the shared front-end phase byte
 *           D_80143BB0 reads 2, raises the shared front-end progress byte
 *           g_ScenarioProgress to 1, runs the GAME.EMI helper func_801BE1B0
 *           with mode 0 and arms secondary state 1; state 1, at progress 8,
 *           arms secondary state 2, hands the scenario flag word D_8014686C and
 *           mode 0x3E to func_8015B580 and runs func_8019FA28 with
 *           0x27, 0x1E0000, 0x270000 and 0x82; state 2 runs func_801C601C with
 *           mode 1, hands the same flag word and mode 3 to func_8015B580 and
 *           arms secondary state 3; state 3, at progress 0x11, raises the
 *           progress byte to 0, arms secondary state 4, seeds the shared
 *           countdown halfword D_80146876 with 0xB4, clears D_8014832E and runs
 *           func_8019FA28 with 0x27, 0x598000, 0x100000 and 0x82; state 4
 *           decrements that countdown on every call and, once the value it read
 *           was already 0, runs func_8014ECAC with mode 5, writes 0x1F into
 *           D_8014832E and arms secondary state 7; state 7, while the shared
 *           halfword D_80143C40 reads 0, raises the progress byte to 1 and arms
 *           secondary state 0x0A; state 0x0A, at progress 2 with D_80149332
 *           clear, hands the flag word and mode 0x3E to func_8015B5A8, hands it
 *           mode 3 to func_8015B580, runs the release helper func_8015C058 and
 *           clears the progress byte and both state bytes. States 5, 6, 8 and 9
 *           have no body. The 0x18-byte frame keeps $ra and $s0 across the
 *           calls, the state-byte load and its bound test are hoisted above the
 *           frame setup, and the eleven-entry case table sits in the overlay
 *           rodata slot at 0x801F6C3C.
 *           Representation note: states 3 and 0x0A read and clear the same
 *           progress byte around a call, so both take a local pointer to it, and
 *           state 4 addresses the countdown halfword through a local pointer so
 *           one base register serves its load and its store; the first seed
 *           spelled all three as plain globals with no rodata placement and
 *           measured 114 of 138 instructions at 556 bytes, while this pointer
 *           spelling with the eleven-entry table placed at 0x801F6C3C measured
 *           138 of 138 at 552 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState1ScenarioScena0600_801F7A44(
    void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 1u;
      func_801BE1B0(0);
      D_80146875 = 1u;
      break;

    case 1:
      if (g_ScenarioProgress != 8u) {
        break;
      }

      D_80146875 = 2u;
      func_8015B580(D_8014686C, 0x3E);
      func_8019FA28(0x27u, 0x1E0000, 0x270000, 0x81u);
      break;

    case 2:
      func_801C601C(1u);
      func_8015B580(D_8014686C, 3);
      D_80146875 = 3u;
      break;

    case 3: {
      u8* progress = &g_ScenarioProgress;

      if (*progress != 0x11u) {
        break;
      }

      *progress = 0u;
      D_80146875 = 4u;
      D_80146876 = 0xB4;
      D_8014832E = 0u;
      func_8019FA28(0x27u, 0x598000, 0x100000, 0x82u);
      break;
    }

    case 4: {
      u16* counter = &D_80146876;
      u16 remaining = *counter;

      *counter = remaining - 1;
      if (remaining != 0) {
        break;
      }

      func_8014ECAC(5u);
      D_8014832E = 0x1F;
      D_80146875 = 7u;
      break;
    }

    case 7:
      if (D_80143C40 != 0) {
        break;
      }

      g_ScenarioProgress = 1u;
      D_80146875 = 0xAu;
      break;

    case 10: {
      u8* progress = &g_ScenarioProgress;

      if (*progress != 2u) {
        break;
      }
      if (D_80149332 != 0) {
        break;
      }

      func_8015B5A8(D_8014686C, 0x3E);
      func_8015B580(D_8014686C, 3);
      func_8015C058();
      *progress = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
    }
  }
}
