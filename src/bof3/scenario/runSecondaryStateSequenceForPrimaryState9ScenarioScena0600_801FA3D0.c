#include "bof3/bof3.h"

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

void func_801BE1B0(u32 arg0);
void func_8014ECAC(u16 arg0);
void func_801C16B8(void);
void func_801C7A54(s32 arg0);
void func_80150224(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u32 context_kind);

/* @source 0x801FA3D0
 * @behavior Primary state 9 of this overlay, reached as word 9 of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through a twenty-four-entry
 *           case table, so every value of that byte at or above 0x18 falls to
 *           the epilogue, as do the unclaimed states 4, 6, 8, 9, 11, 12, 13,
 *           14 and 19. State 0 raises the shared front-end progress byte
 *           g_ScenarioProgress to 1 and arms secondary state 1 unless the
 *           shared front-end phase byte D_80143BB0 reads 2; state 2 raises that
 *           progress byte to 1 in the same way and arms secondary state 3.
 *           States 1 and 3 share one body: at progress 0x0A it arms secondary
 *           state 5, sets flag bit 0x1D of the shared scenario flag word
 *           D_8014686C through func_8015B580, raises D_80146866 to 4, sets bit
 *           0 of the shared front state byte D_80146871, runs func_8019FA28
 *           with selector seed 0x5C, context words 0x180000 and 0x3D0000 and
 *           kind 0x89, and then stores 0x3C in the shared front selection byte
 *           D_80143F1F. State 5, at progress 0x0F, runs func_801C7A54 with mode
 *           0x1B and advances to secondary state 6. State 7 runs func_801BE1B0
 *           with mode 0x0E, arms secondary state 0x0A, sets flag bit 0x1F of
 *           D_8014686C through func_8015B580, runs func_8019FA28 with seed 0x5C,
 *           context words 0x180000 and 0x3C0000 and kind 0x8A, and stores 0xFF
 *           in D_80143F1F. State 10, at progress 0x14, runs func_801C16B8,
 *           raises D_80146866 to 2, arms secondary state 6, toggles bit 0 of
 *           D_80146871 and runs func_8019FA28 with seed 0x57, context words
 *           0x310000 and 0x2D0000 and kind 4. State 15 runs func_801BE1B0 with
 *           mode 0x0B and arms secondary state 0x10. State 16, at progress 0x14,
 *           runs func_8014ECAC with mode 0x0D, sets flag bit 0x20 of D_8014686C
 *           through func_8015B580 and arms secondary state 0x11. State 17, once
 *           the header gate halfword D_80143C40 reads zero, clears the shared
 *           effect bank byte D_8014832E, queues front-end action 0x2B through
 *           func_80150224, latches the phase byte D_80143BB0 to 2 and arms
 *           secondary state 0x12. State 20 runs func_801BE1B0 with mode 0x0D and
 *           arms secondary state 0x15. State 21, at progress 2, runs
 *           func_8014ECAC with mode 0x0D, sets flag bit 0x20 of D_8014686C
 *           through func_8015B580 and arms secondary state 0x16. State 22, once
 *           D_80143C40 reads zero, clears D_8014832E, queues front-end action
 *           0x29 through func_80150224, latches D_80143BB0 to 2 and arms
 *           secondary state 0x17. States 18 and 23 share one body: unless
 *           D_80143BB0 already reads 2 it runs func_801C16B8, raises D_80146866
 *           to 1, arms secondary state 6, passes D_8014686C and bit 0x1D to
 *           func_8015B5A8, toggles bit 0 of D_80146871 and runs the same
 *           func_8019FA28 request as state 10. The 0x18-byte frame keeps $ra
 *           across the calls and the state-byte load and its bound test are
 *           hoisted above the frame setup; the twenty-four-word case table sits
 *           at 0x801F6FC4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState9ScenarioScena0600_801FA3D0(
    void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 1u;
      D_80146875 = 1u;
      break;

    case 2:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 1u;
      D_80146875 = 3u;
      break;

    case 1:
    case 3:
      if (g_ScenarioProgress == 0xAu) {
        D_80146875 = 5u;
        func_8015B580(D_8014686C, 0x1D);
        D_80146866 = 4u;
        D_80146871 |= 1;
        func_8019FA28(0x5Cu, 0x180000u, 0x3D0000u, 0x89u);
        D_80143F1F = 0x3Cu;
      }
      break;

    case 5:
      if (g_ScenarioProgress == 0xFu) {
        func_801C7A54(0x1B);
        D_80146875 = 6u;
      }
      break;

    case 7:
      func_801BE1B0(0xEu);
      D_80146875 = 0xAu;
      func_8015B580(D_8014686C, 0x1F);
      func_8019FA28(0x5Cu, 0x180000u, 0x3C0000u, 0x8Au);
      D_80143F1F = 0xFFu;
      break;

    case 10:
      if (g_ScenarioProgress == 0x14u) {
        func_801C16B8();
        D_80146866 = 2u;
        D_80146875 = 6u;
        D_80146871 ^= 1;
        func_8019FA28(0x57u, 0x310000u, 0x2D0000u, 4u);
      }
      break;

    case 15:
      func_801BE1B0(0xBu);
      D_80146875 = 0x10u;
      break;

    case 16:
      if (g_ScenarioProgress == 0x14u) {
        func_8014ECAC(0xDu);
        func_8015B580(D_8014686C, 0x20);
        D_80146875 = 0x11u;
      }
      break;

    case 17:
      if (D_80143C40 == 0) {
        D_8014832E = 0u;
        func_80150224(0x2B);
        D_80143BB0 = 2u;
        D_80146875 = 0x12u;
      }
      break;

    case 20:
      func_801BE1B0(0xDu);
      D_80146875 = 0x15u;
      break;

    case 21:
      if (g_ScenarioProgress == 2u) {
        func_8014ECAC(0xDu);
        func_8015B580(D_8014686C, 0x20);
        D_80146875 = 0x16u;
      }
      break;

    case 22:
      if (D_80143C40 == 0) {
        D_8014832E = 0u;
        func_80150224(0x29);
        D_80143BB0 = 2u;
        D_80146875 = 0x17u;
      }
      break;

    case 18:
    case 23:
      if (D_80143BB0 == 2u) {
        break;
      }

      func_801C16B8();
      D_80146866 = 1u;
      D_80146875 = 6u;
      func_8015B5A8(D_8014686C, 0x1D);
      D_80146871 ^= 1;
      func_8019FA28(0x57u, 0x310000u, 0x2D0000u, 4u);
      break;
  }
}
