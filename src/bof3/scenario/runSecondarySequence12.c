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
/* Front-end phase byte of the shared main-executable state block. */
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* Shared gate halfword: every handler that waits on it runs only while it
 * reads zero.
 * @source 0x80143C40 @kind unknown
 */
extern u16 D_80143C40;
/* Shared effect-bank byte armed with 0x1F by the handlers that start a bank.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;
/* Shared decision halfword; bit 0x2 is the advancing decision here.
 * @source 0x801490A4 @kind unknown
 */
extern u16 D_801490A4;
/* Signed shared byte written 0 by state 8 of this body and tested by state 10;
 * the original loads it with a signed byte load.
 * @source 0x8014865F @kind unknown
 */
extern s8 D_8014865F;
/* Shared byte latched to 1 by state 11 and tested by state 12. */
/* @source 0x80148660 @kind unknown */
extern u8 D_80148660;
/* Shared hint byte; only its low seven bits select the inner state-17 case. */
/* @source 0x80145024 @kind unknown */
extern u8 D_80145024;
/* Shared index byte that selects the 0xA4-byte record this body patches. */
/* @source 0x80181B12 @kind unknown */
extern u8 D_80181B12;
/* Base of the shared 0xA4-byte record run: the original re-materialises
 * `lui at,0x8014` for the load and again for the store, so the record field is
 * addressed through its own address-derived symbol.
 * @source 0x8014496F @kind unknown
 */
extern u8 D_8014496F[];

void publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80(u8 step);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
void func_8015D404(u32 arg0, s32 arg1);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
void func_8014ECAC(u16 arg0);
void func_8015D300(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
void func_80150224(s32 action_id);
void func_801667A4(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_801C1E3C(u8 index);
void func_801C1DF0(u8 index);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_8015C058(void);

/* @source 0x801FAB08
 * @behavior Primary state 12 of this overlay, reached as word 12 of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateTable indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through a twenty-entry case
 *           table in which state 9 has no case and falls to the epilogue:
 *           state 0 claims front-end record 0x91 through
 *           publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80 and
 *           arms state 1; state 1 runs func_801BE1B0 and func_801C601C with
 *           mode 0 and arms state 2; states 2 to 6 each wait for one progress
 *           value (1, 3, 5, 0x0A, 0x1D) and then advance one state after
 *           releasing the front-end record or the queued action that value
 *           gates. State 7, once the gate halfword D_80143C40 reads zero,
 *           clears the effect bank D_8014832E, sets progress 0x1F, releases the
 *           front-end record through func_8015D404 and starts the shared timing
 *           through func_8015D300, queues action 0x0D, latches the phase byte
 *           D_80143BB0 to 2 and arms state 8. State 8, once bit 0x2 of the
 *           decision halfword D_801490A4 is set, runs func_801667A4, latches the
 *           phase byte to 6, clears the signed byte D_8014865F and arms state
 *           0x0A. State 0x0A advances to state 0x0B only while that signed byte
 *           reads nonzero. State 0x0B clears bit 0x2 of the record at
 *           D_8014496F + 0xA4 * D_80181B12, latches the phase byte to 1 and
 *           D_80148660 to 1 and arms state 0x0C. State 0x0C, while D_80148660 is
 *           clear, arms the effect bank with 0x1F, runs func_801BFE34 and queues
 *           action 0x45, then falls through into state 0x0D, which advances to
 *           state 0x0E at progress 0x28 while the phase byte still reads zero.
 *           State 0x0E waits for progress 0x3C and releases front-end record
 *           0x18; state 0x0F waits for progress 5, runs func_801BE1B0 with mode
 *           3, sets flag 0 of D_8014686C through func_8015B580, arms state
 *           0x10 and then releases front-end record 0x0D. State 0x10 waits on
 *           the gate halfword before clearing the effect bank and arming state
 *           0x11, and state 0x11 sets flag 1 of D_8014686C, arms state 0x12 and
 *           dispatches on the low seven bits of D_80145024:
 *           selector 0 patches no record and calls func_8019FA28 alone,
 *           selector 3 clears D_80146866 and releases record 6 first, and
 *           selector 4 does the same through record 4, after which both release
 *           record 6 and call func_8019FA28. State 0x12 arms the effect bank
 *           with 0x1F and advances to state 0x13, which at progress 0x14 queues
 *           action 0x44, clears the shared bytes 0x80146865 to 0x80146867,
 *           releases the overlay through func_8015C058 and clears both the
 *           secondary and the primary state bytes. The 0x18-byte frame keeps
 *           only $ra across the calls, the state-byte load and its bound test
 *           are hoisted above the frame setup, and the twenty-word case table
 *           sits at 0x801F706C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondarySequence12(
    void) {
  switch (D_80146875) {
    case 0:
      publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80(0x91);
      D_80146875 = 1u;
      break;

    case 1:
      func_801BE1B0(0u);
      func_801C601C(0u);
      D_80146875 = 2u;
      break;

    case 2:
      if (g_ScenarioProgress == 1u) {
        func_8015D404(0u, 0);
        func_80161C20(0x45, 0x64, 8);
        D_80146875 = 3u;
      }
      break;

    case 3:
      if (g_ScenarioProgress == 3u) {
        func_801BFE34(0x1D0000, 0x1D0000, 0x17);
        D_80146875 = 4u;
      }
      break;

    case 4:
      if (g_ScenarioProgress == 5u) {
        func_801C7A54(0x17);
        D_80146875 = 5u;
      }
      break;

    case 5:
      if (g_ScenarioProgress == 0x0A) {
        func_801BE1B0(1u);
        D_80146875 = 6u;
      }
      break;

    case 6:
      if (g_ScenarioProgress == 0x1D) {
        func_8014ECAC(4);
        D_80146875 = 7u;
      }
      break;

    case 7:
      if (D_80143C40 == 0) {
        D_8014832E = 0u;
        g_ScenarioProgress = 0x1F;
        func_8015D404(0u, 1);
        func_8015D300(0u, 2u, 0x64, 0x64);
        func_80150224(0x0D);
        D_80143BB0 = 2u;
        D_80146875 = 8u;
      }
      break;

    case 8:
      if (D_801490A4 & 2) {
        func_801667A4(2);
        D_80143BB0 = 6u;
        D_8014865F = 0;
        D_80146875 = 0x0A;
      }
      break;

    case 10:
      if (D_8014865F != 0) {
        D_80146875 = 0x0B;
      }
      break;

    case 11: {
      s32 offset;

      offset = D_80181B12 * 0xA4;
      D_8014496F[offset] &= 0xFD;
      D_80143BB0 = 1u;
      D_80148660 = 1u;
      D_80146875 = 0x0C;
      break;
    }

    case 12:
      if (D_80148660 == 0u) {
        D_8014832E = 0x1F;
        func_801BFE34(0x1D0000, 0x1D0000, 0x18);
        func_80161C20(0x45, 0x64, 8);
        D_80146875 = 0x0D;
      }
      /* Falls through into the state 0x0D arm the original shares with it. */
    case 13:
      if (D_80143BB0 == 0u) {
        g_ScenarioProgress = 0x28;
        D_80146875 = 0x0E;
      }
      break;

    case 14:
      if (g_ScenarioProgress == 0x3C) {
        func_801C7A54(0x18);
        D_80146875 = 0x0F;
      }
      break;

    case 15:
      if (g_ScenarioProgress == 5u) {
        func_801BE1B0(3u);
        func_8015B580(D_8014686C, 0);
        D_80146875 = 0x10;
        func_8014ECAC(0x0D);
      }
      break;

    case 16:
      if (D_80143C40 == 0) {
        D_8014832E = 0u;
        D_80146875 = 0x11;
      }
      break;

    case 17:
      D_80146875 = 0x12;
      func_8015B580(D_8014686C, 1);
      switch (D_80145024 & 0x7F) {
        case 0:
          D_80146866 = 0u;
          func_8019FA28(0x43, 0x1D0000, 0x1C0000, 0x82);
          break;

        case 3:
          D_80146866 = 0u;
          func_801C1E3C(6);
          func_801C1DF0(6);
          func_8019FA28(0x43, 0x1D0000, 0x1C0000, 0x82);
          break;

        case 4:
          D_80146866 = 0u;
          func_801C1E3C(4);
          func_801C1DF0(6);
          func_8019FA28(0x43, 0x1D0000, 0x1C0000, 0x82);
          break;
      }
      break;

    case 18:
      D_8014832E = 0x1F;
      D_80146875 = 0x13;
      break;

    case 19:
      if (g_ScenarioProgress == 0x14) {
        func_80161C20(0x44, 0x64, 8);
        D_80146865 = 0u;
        D_80146866 = 0u;
        D_80146867 = 0u;
        func_8015C058();
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;
  }
}
