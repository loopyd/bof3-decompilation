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
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801C16B8(void);

/* @source 0x801F8F18
 * @behavior Primary state 5 of this overlay: word 5 (0-based) of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874, so a primary state
 *           reading 5 runs it; no jal of the payload names 0x801F8F18 and that
 *           little-endian table word is its only reference. It switches on the
 *           shared secondary scene state byte D_80146875 (read with lbu)
 *           through the six-word absolute case table at 0x801F6D7C, bounded by
 *           `sltiu` against 6, and every state falls through to one epilogue
 *           that restores $ra from a 0x18-byte frame; the state-byte load and
 *           its bound test are hoisted above the frame setup.
 *           State 0 requests front-end mode 0x0D through func_8014ECAC and
 *           advances to state 1. State 1, while the shared halfword D_80143C40
 *           reads zero, clears D_8014832E, advances to state 2 and seeds the
 *           front-selector context (0x35, 0x120000, 0x180000, 0x81) through
 *           func_8019FA28. State 2, at progress 0x0C, requests the same mode,
 *           runs func_801C16B8, advances to state 3, hands the scenario flag
 *           word D_8014686C and mode 0x11 to func_8015B580 and starts the
 *           scripted motion func_8015D554 with (0, 0, 4, 0x1E). State 3, while
 *           D_80143C40 reads zero, clears D_8014832E, runs func_801636A0(0, 1)
 *           and advances to state 4. State 4, once func_80163EA0 reports a
 *           nonzero low byte, advances to state 5 and seeds the context
 *           (0x57, 0x310000, 0x2E0000, 3). State 5 stores 0x1F in D_8014832E,
 *           runs the shared overlay release helper func_8015C058 and clears
 *           D_80146865, D_80146866, D_80146867 and the secondary and primary
 *           state bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState5ScenarioScena0600_801F8F18(void) {
  switch (D_80146875) {
    case 0:
      func_8014ECAC(0xDu);
      D_80146875 = 1u;
      break;

    case 1:
      if (D_80143C40 != 0u) {
        break;
      }

      D_8014832E = 0u;
      D_80146875 = 2u;
      func_8019FA28(0x35u, 0x120000u, 0x180000u, 0x81u);
      break;

    case 2:
      if (g_ScenarioProgress != 0xCu) {
        break;
      }

      func_8014ECAC(0xDu);
      func_801C16B8();
      D_80146875 = 3u;
      func_8015B580(D_8014686C, 0x11);
      func_8015D554(0, 0, 4, 0x1E);
      break;

    case 3:
      if (D_80143C40 != 0u) {
        break;
      }

      D_8014832E = 0u;
      func_801636A0(0u, 1u);
      D_80146875 = 4u;
      break;

    case 4:
      if ((u8)func_80163EA0() == 0u) {
        break;
      }

      D_80146875 = 5u;
      func_8019FA28(0x57u, 0x310000u, 0x2E0000u, 3u);
      break;

    case 5:
      D_8014832E = 0x1Fu;
      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
  }
}
