#include "bof3/bof3.h"

/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
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
/* Front-end phase byte the shared main-executable state block carries next to
 * the scene state bytes.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;
/* Front-end record table: the claim arm writes byte 0 and byte +5 of the
 * 0x74-byte row its claimed index selects, through one address computation.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Shared flag bank that the state 4 and state 6 arms and the reset arms of
 * states 7 and 8 address directly.
 * @source 0x80144F28 @kind unknown
 */
extern u8 D_80144F28;

void func_80150224(s32 arg0);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015DF18(u32 arg0);
void func_801A4BC0(s32 arg0, s32 arg1, s32 arg2);
void func_801BE1B0(u32 arg0);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
u8 func_8019601C(void);

/* @source 0x801FB014
 * @behavior Primary state 13 of this overlay, reached as word 13 (0-based) of
 *           the 18-word primary state table at 0x801FE3E4, which
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874. It steps the
 *           shared secondary scene state byte D_80146875 through a nine-entry
 *           case table: every value above 8 falls to the epilogue. State 0
 *           requests func_80150224 mode 0xC, latches the shared front-end
 *           phase byte D_80143BB0 to 2 and arms secondary state 1; state 1
 *           runs only while that phase byte reads anything but 2, arms
 *           func_801A4BC0(0x5D, 0x2E, 0), claims one front-end record through
 *           func_8019601C and, unless the claim reads 0xFF, writes 1 to byte 0
 *           and 0x45 to byte +5 of the 0x74-byte row the claimed index selects
 *           in the table at 0x80143FC8, then publishes the shared flag word
 *           D_8014686C to func_8015B5A8 mode 0x26 and func_8015B580 mode 0x2D
 *           before arming secondary state 2; state 2 advances to 3 once
 *           func_8015B5D4 mode 0x2D reads zero; state 3 arms secondary state 4
 *           while that predicate with mode 0x26 is non-zero and 6 otherwise;
 *           state 4 arms func_801A4BC0(0x5D, 0x2E, 0x51), publishes the shared
 *           flag bank at 0x80144F28 through func_8015B580 mode 0x2F, runs
 *           func_8015DF18 mode 0x201 and func_801BE1B0 mode 1 and arms
 *           secondary state 5; state 5, at progress 1, publishes D_8014686C
 *           with mode 0x27, runs func_8015DF18 mode 0x204, queues
 *           func_80161C20(0x5B, 0x64, 8) and joins the reset tail; state 6
 *           arms func_801A4BC0(0x5D, 0x2E, 0x51) and either arms secondary
 *           state 7, or, when mode 0x25 is answered, publishes the flag bank
 *           with mode 0x2F, runs func_8015DF18 modes 0x201 and 0x204, queues
 *           the same 0x5B/0x64/8 cue, runs func_801BE1B0 mode 2, publishes
 *           D_8014686C with mode 0x25 and arms secondary state 8; state 7
 *           publishes the flag bank with mode 0x2F and clears the progress
 *           byte; state 8 does the same at progress 1. States 5, 7 and 8 join
 *           one reset tail that clears D_80146865..D_80146867, runs
 *           func_8015C058 and clears both scene state bytes. The 0x18-byte
 *           frame keeps only $ra across the calls, the state-byte load and its
 *           bound test are hoisted above the frame setup, and the nine-word
 *           case table sits in the overlay rodata slot at 0x801F70BC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState13ScenarioScena0600_801FB014(void) {
  switch (D_80146875) {
    case 0:
      func_80150224(0xC);
      D_80143BB0 = 2u;
      D_80146875 = 1u;
      break;

    case 1: {
      u8 index;
      u8 *record;

      if (D_80143BB0 == 2u) {
        break;
      }

      func_801A4BC0(0x5D, 0x2E, 0);
      index = func_8019601C();

      if (index != 0xFFu) {
        record = (u8 *)D_80143FC8 + (u32)index * 0x74;
        record[0] = 1u;
        record[5] = 0x45u;
      }

      func_8015B5A8(D_8014686C, 0x26);
      func_8015B580(D_8014686C, 0x2D);
      D_80146875 = 2u;
      break;
    }

    case 2:
      if (func_8015B5D4(D_8014686C, 0x2D) != 0) {
        break;
      }

      D_80146875 = 3u;
      break;

    case 3:
      if (func_8015B5D4(D_8014686C, 0x26) != 0) {
        D_80146875 = 4u;
      } else {
        D_80146875 = 6u;
      }
      break;

    case 4:
      func_801A4BC0(0x5D, 0x2E, 0x51);
      func_8015B580((u32)&D_80144F28, 0x2F);
      func_8015DF18(0x201);
      func_801BE1B0(1u);
      D_80146875 = 5u;
      break;

    case 5:
      if (g_ScenarioProgress != 1u) {
        break;
      }

      func_8015B580(D_8014686C, 0x27);
      func_8015DF18(0x204);
      func_80161C20(0x5B, 0x64, 8);
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 6:
      func_801A4BC0(0x5D, 0x2E, 0x51);

      if (func_8015B5D4(D_8014686C, 0x25) == 0) {
        D_80146875 = 7u;
      } else {
        func_8015B580((u32)&D_80144F28, 0x2F);
        func_8015DF18(0x201);
        func_8015DF18(0x204);
        func_80161C20(0x5B, 0x64, 8);
        func_801BE1B0(2u);
        func_8015B5A8(D_8014686C, 0x25);
        D_80146875 = 8u;
      }
      break;

    case 7:
      func_8015B5A8((u32)&D_80144F28, 0x2F);
      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 8:
      if (g_ScenarioProgress != 1u) {
        break;
      }

      func_8015B5A8((u32)&D_80144F28, 0x2F);
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
  }
}
