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
/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* @source 0x8018E244 @kind unknown */
extern u8 D_8018E244;
/* Shared counter of the first name stream this overlay advances.
 * @source 0x80144DF2 @kind data
 */
extern u8 stateSelectionCounter;
/* Shared counter of the second name stream this overlay advances.
 * @source 0x80144DF5 @kind data
 */
extern u8 stateSelectionCounter2;

void func_80150224(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8016666C(s32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u32 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C16B8(void);
void func_801C601C(u32 arg0);

/* @source 0x801F97C0
 * @behavior Primary state 7 of this overlay: word 7 (0-based) of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateTable indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through a sixty-three-entry
 *           case table, so every value of that byte at or above 0x3F falls to
 *           the epilogue, as do the unclaimed states 7 to 9, 12, 13, 16, 17,
 *           22 to 24, 34, 38, 39, 41 to 44, 46 to 49 and 52 to 59. State 0 arms
 *           secondary state 1 unless the front-end phase byte D_80143BB0 reads
 *           2. State 1 raises D_80146866 to 1, arms secondary state 2, sets bit
 *           0 of D_80146871, runs func_8019FA28 with 0x5C, 0x0E0000, 0x3C0000
 *           and 0x80 and stores 0xFF in D_80143F1F. State 2 queues front-end
 *           action 1 through func_80150224, latches D_80143BB0 to 2 and arms
 *           secondary state 3. State 3, unless D_80143BB0 already reads 2,
 *           stores 0x1F in D_8014832E, runs func_801C601C with mode 0 and arms
 *           secondary state 4. State 4, at progress 2, latches D_80143BB0 to 7,
 *           clears D_8018E244 and D_8014832E and arms secondary state 5. State
 *           5, while D_80143BB0 reads zero, arms secondary state 6. State 6 arms
 *           0x0E while the shared byte D_80144DF2 reads zero, then 0x0A while
 *           D_80144DF5 reads nonzero and 0x0E otherwise. State 0x0A raises
 *           D_80146866 to 2, arms secondary state 0x0B, hands the flag word
 *           D_8014686C and modes 0x1A and 0x1B to func_8015B580, runs
 *           func_8019FA28 with 0x5C, 0x190000, 0x3C0000 and 0x81 and stores
 *           0x3C in D_80143F1F. States 0x0B, 0x0F, 0x21, 0x2D and 0x33 share one
 *           body: at progress 3 it runs func_8015C058 and enters the clearing
 *           tail, which zeroes D_80146865 to D_80146867, D_80146875 and
 *           D_80146874. State 0x0E raises D_80146866 to 2, arms secondary state
 *           0x0F, hands D_8014686C and mode 0x17 to func_8015B580 and runs
 *           func_8019FA28 with 0x5C, 0x190000, 0x3C0000 and 0x82. State 0x12,
 *           unless D_80143BB0 already reads 2, raises progress to 1, hands
 *           D_8014686C and mode 0x1B to func_8015B580 and arms secondary state
 *           0x13. State 0x13, at progress 2, runs func_8016666C with (1000, 0),
 *           raises progress to 3 and arms secondary state 0x14. States 0x14 and
 *           0x24 share one body, which arms the next state, runs func_801C16B8,
 *           toggles bit 0 of D_80146871, hands D_8014686C and modes 0x15 and
 *           0x17 to func_8015B5A8 and runs func_8019FA28 with 0x57, 0x310000,
 *           0x2D0000 and 4; state 0x14 enters it at progress 4 and state 0x24 at
 *           progress 3. States 0x15 and 0x25 run func_8015C058, zero progress and
 *           enter that same clearing tail. State 0x19, unless D_80143BB0 already
 *           reads 2, arms secondary state 0x1A. State 0x1A, on that same test,
 *           clears D_8014832E, arms secondary state 0x1B, sets bit 0 of
 *           D_80146871 and runs func_8019FA28 with 0x5C, 0x0E0000, 0x3C0000 and
 *           0x80. State 0x1B queues front-end action 1 through func_80150224,
 *           latches D_80143BB0 to 2 and arms secondary state 0x1C. State 0x1C,
 *           unless D_80143BB0 already reads 2, stores 0x1F in D_8014832E, runs
 *           func_801C601C with mode 0 and arms secondary state 0x1D. State 0x1D,
 *           at progress 2, latches D_80143BB0 to 7, clears D_8018E244 and
 *           D_8014832E and arms secondary state 0x1E. State 0x1E, while
 *           D_80143BB0 reads zero, arms secondary state 0x1F. State 0x1F arms
 *           0x20 while D_80144DF2 or D_80144DF5 reads zero and 0x28 otherwise.
 *           State 0x20 raises D_80146866 to 2, arms secondary state 0x21, hands
 *           D_8014686C and mode 0x17 to func_8015B580 and runs func_8019FA28
 *           with 0x5C, 0x190000, 0x3C0000 and 0x82. State 0x23, unless
 *           D_80143BB0 already reads 2, raises progress to 1, arms secondary
 *           state 0x24 and runs func_801BE1B0 with mode 3. State 0x28 arms
 *           0x2D, raises D_80146866 to 2, hands D_8014686C and modes 0x1A and
 *           0x1B to func_8015B580, runs func_8019FA28 with 0x5C, 0x190000,
 *           0x3C0000 and 0x81 and stores 0x3C in D_80143F1F. State 0x32, unless
 *           D_80143BB0 already reads 2, arms secondary state 0x33, raises
 *           D_80146866 to 1, sets bit 0 of D_80146871 and runs func_8019FA28
 *           with 0x5C, 0x190000, 0x3C0000 and 0x88. State 0x3C, unless
 *           D_80143BB0 already reads 2, latches it to 7, clears D_8018E244 and
 *           D_8014832E and arms secondary state 0x3D. State 0x3D, while
 *           D_80143BB0 reads zero, arms secondary state 0x3E. State 0x3E arms
 *           0x20 while D_80144DF2 reads zero, 0x28 while D_80144DF5 reads
 *           nonzero and 0x20 otherwise. The 0x18-byte frame keeps $ra and $s0,
 *           which holds the progress byte address of state 0x13, and the
 *           sixty-three-word case table sits in the overlay rodata slot at
 *           0x801F6E1C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState7Phases(void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_80146875 = 1u;
      return;

    case 1:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_80146866 = 1u;
      D_80146875 = 2u;
      D_80146871 |= 1;
      func_8019FA28(0x5Cu, 0xE0000u, 0x3C0000u, 0x80u);
      D_80143F1F = 0xFFu;
      return;

    case 2:
      func_80150224(1);
      D_80143BB0 = 2u;
      D_80146875 = 3u;
      return;

    case 3:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_8014832E = 0x1Fu;
      func_801C601C(0);
      D_80146875 = 4u;
      return;

    case 4:
      if (g_ScenarioProgress != 2u) {
        return;
      }

      D_80143BB0 = 7u;
      D_8018E244 = 0u;
      D_8014832E = 0u;
      D_80146875 = 5u;
      return;

    case 5:
      if (D_80143BB0 != 0u) {
        return;
      }

      D_80146875 = 6u;
      return;

    case 6:
      if (stateSelectionCounter == 0u) {
        D_80146875 = 0xEu;
        return;
      }

      if (stateSelectionCounter2 != 0u) {
        D_80146875 = 0xAu;
        return;
      }

      D_80146875 = 0xEu;
      return;

    case 10:
      D_80146866 = 2u;
      D_80146875 = 0xBu;
      func_8015B580(D_8014686C, 0x1A);
      func_8015B580(D_8014686C, 0x1B);
      func_8019FA28(0x5Cu, 0x190000u, 0x3C0000u, 0x81u);
      D_80143F1F = 0x3Cu;
      return;

    case 11:
      if (g_ScenarioProgress != 3u) {
        return;
      }

      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 14:
      D_80146866 = 2u;
      D_80146875 = 0xFu;
      func_8015B580(D_8014686C, 0x17);
      func_8019FA28(0x5Cu, 0x190000u, 0x3C0000u, 0x82u);
      return;

    case 15:
      if (g_ScenarioProgress != 3u) {
        return;
      }

      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 18:
      if (D_80143BB0 == 2u) {
        return;
      }

      g_ScenarioProgress = 1u;
      func_8015B580(D_8014686C, 0x1B);
      D_80146875 = 0x13u;
      return;

    case 19: {
      u8 *progress = &g_ScenarioProgress;

      if (*progress != 2u) {
        return;
      }

      func_8016666C(1000, 0);
      *progress = 3u;
      D_80146875 = 0x14u;
      return;
    }

    case 20:
      if (g_ScenarioProgress != 4u) {
        return;
      }

      D_80146875 = 0x15u;
      func_801C16B8();
      D_80146871 ^= 1;
      func_8015B5A8(D_8014686C, 0x15);
      func_8015B5A8(D_8014686C, 0x17);
      func_8019FA28(0x57u, 0x310000u, 0x2D0000u, 4u);
      return;

    case 21:
      func_8015C058();
      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 25:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_80146875 = 0x1Au;
      return;

    case 26:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_8014832E = 0u;
      D_80146875 = 0x1Bu;
      D_80146871 |= 1;
      func_8019FA28(0x5Cu, 0xE0000u, 0x3C0000u, 0x80u);
      return;

    case 27:
      func_80150224(1);
      D_80143BB0 = 2u;
      D_80146875 = 0x1Cu;
      return;

    case 28:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_8014832E = 0x1Fu;
      func_801C601C(0);
      D_80146875 = 0x1Du;
      return;

    case 29:
      if (g_ScenarioProgress != 2u) {
        return;
      }

      D_80143BB0 = 7u;
      D_8018E244 = 0u;
      D_8014832E = 0u;
      D_80146875 = 0x1Eu;
      return;

    case 30:
      if (D_80143BB0 != 0u) {
        return;
      }

      D_80146875 = 0x1Fu;
      return;

    case 31:
      if (stateSelectionCounter == 0u) {
        D_80146875 = 0x20u;
        return;
      }

      if (stateSelectionCounter2 == 0u) {
        D_80146875 = 0x20u;
        return;
      }

      D_80146875 = 0x28u;
      return;

    case 32:
      D_80146866 = 2u;
      D_80146875 = 0x21u;
      func_8015B580(D_8014686C, 0x17);
      func_8019FA28(0x5Cu, 0x190000u, 0x3C0000u, 0x82u);
      return;

    case 33:
      if (g_ScenarioProgress != 3u) {
        return;
      }

      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 35:
      if (D_80143BB0 == 2u) {
        return;
      }

      g_ScenarioProgress = 1u;
      D_80146875 = 0x24u;
      func_801BE1B0(3u);
      return;

    case 36:
      if (g_ScenarioProgress != 3u) {
        return;
      }

      D_80146875 = 0x25u;
      func_801C16B8();
      D_80146871 ^= 1;
      func_8015B5A8(D_8014686C, 0x15);
      func_8015B5A8(D_8014686C, 0x17);
      func_8019FA28(0x57u, 0x310000u, 0x2D0000u, 4u);
      return;

    case 37:
      func_8015C058();
      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 40:
      D_80146875 = 0x2Du;
      D_80146866 = 2u;
      func_8015B580(D_8014686C, 0x1A);
      func_8015B580(D_8014686C, 0x1B);
      func_8019FA28(0x5Cu, 0x190000u, 0x3C0000u, 0x81u);
      D_80143F1F = 0x3Cu;
      return;

    case 45:
      if (g_ScenarioProgress != 3u) {
        return;
      }

      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 50:
      if (D_80143BB0 == 2u) {
        return;
      }

      D_80146875 = 0x33u;
      D_80146866 = 1u;
      D_80146871 |= 1;
      func_8019FA28(0x5Cu, 0x190000u, 0x3C0000u, 0x88u);
      return;

    case 51:
      if (g_ScenarioProgress != 3u) {
        return;
      }

      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 60: {
      u8 *phase = &D_80143BB0;

      if (*phase == 2u) {
        return;
      }

      *phase = 7u;
      D_8018E244 = 0u;
      D_8014832E = 0u;
      D_80146875 = 0x3Du;
      return;
    }

    case 61:
      if (D_80143BB0 != 0u) {
        return;
      }

      D_80146875 = 0x3Eu;
      return;

    case 62:
      if (stateSelectionCounter == 0u) {
        D_80146875 = 0x20u;
        return;
      }

      if (stateSelectionCounter2 != 0u) {
        D_80146875 = 0x28u;
        return;
      }

      D_80146875 = 0x20u;
      return;

    default:
      return;
  }
}
