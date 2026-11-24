#include "bof3/bof3.h"

/* Shared secondary scene state byte: the sequence step this body advances.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;
/* Shared primary scene state byte: the table index that reaches this word.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;
/* Shared front-end progress byte gating most of the states below.
 * @source 0x80146864 @kind unknown
 */
extern u8 g_ScenarioProgress;
/* Shared byte the reset tail clears before the overlay release helper runs.
 * @source 0x80146865 @kind unknown
 */
extern u8 D_80146865;
/* Shared byte the reset tail clears before the overlay release helper runs.
 * @source 0x80146866 @kind unknown
 */
extern u8 D_80146866;
/* Shared byte that carries the claimed front-end row index between the claim
 * arms of states 0x0C, 0x0D and 0x0E.
 * @source 0x80146867 @kind unknown
 */
extern u8 D_80146867;
/* Shared scenario flag word the late arms publish through func_8015B580.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;
/* Shared front-end phase byte: 2 means the front end already owns the frame.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;
/* Shared gate halfword of the front-end mode: the arms of states 0x0A and 0x0B
 * run only while it reads zero.
 * @source 0x80143C40 @kind unknown
 */
extern u16 D_80143C40;
/* Shared flag bank that state 6 publishes through func_8015B580 mode 7.
 * @source 0x80144F28 @kind unknown
 */
extern u8 D_80144F28;
/* The shared three-byte front-end selection array: the three scan arms of
 * state 0x14 walk it as a table.
 * @source 0x80144F5A @kind unknown
 */
extern u8 D_80144F5A[];
/* Front-end record table and the field bases of its 0x74-byte rows: the
 * original re-materialises the page immediate for every store, so each field
 * is addressed as its own absolute symbol rather than through one struct base.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FD1[];
/* @source 0x8014402C @kind unknown */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind unknown */
extern u8 D_80144030[];
/* @source 0x80144034 @kind unknown */
extern u8 D_80144034[];
/* Scratchpad claim cell the front-end row arms store the claimed index in and
 * re-read for every field they seed.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000;
/* Shared effect cue byte state 6 arms.
 * @source 0x8014933B @kind unknown
 */
extern u8 D_8014933B;
/* Signed source halfwords the row arms copy into the claimed row fields +0x68
 * and +0x6C.
 * @source 0x801492DA @kind unknown
 */
extern s16 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern s16 D_801492DC;

void func_8014ECAC(u16 local_mode);
void func_80150224(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015D404(u32 arg0, s32 arg1);
void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8015DF18(u32 arg0);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
u8 func_8019601C(void);

/* @source 0x801FBD88
 * @behavior Word 15 (0-based) of the 18-word primary state table at
 *           0x801FE3E4, which dispatchPrimaryStateTable
 *           indexes with the shared primary scene state byte D_80146874. It
 *           steps the shared secondary scene state byte D_80146875 through a
 *           31-entry case table in which only states 0, 1, 5, 6, 9, 10, 11, 12,
 *           13, 14, 17, 18, 19, 20, 21, 22 and 30 have a body; every other
 *           value, and every value above 0x1E, falls to the epilogue. State 0
 *           queues front-end action 0x1A through func_80150224, latches the
 *           phase byte D_80143BB0 to 2 and arms secondary state 1; state 1,
 *           unless that phase byte already reads 2, clears the progress byte
 *           and joins the reset tail; state 5 queues action 0x1B, latches the
 *           phase byte to 2 and arms secondary state 6; state 6, unless the
 *           phase byte reads 2, claims one front-end record through
 *           func_8019601C and, unless the claim reads 0xFF, writes 1 to byte 0
 *           and 0x4B to byte +5 of the 0x74-byte row the claim selects, then
 *           runs func_801BE1B0 mode 6, arms the effect cue byte D_8014933B to
 *           1, publishes the shared flag bank 0x80144F28 through func_8015B580
 *           mode 7, queues cues 0x207 and 0x201 and arms secondary state 9;
 *           state 9, at progress 0x29, runs func_8014ECAC mode 0x0F and arms
 *           secondary state 0x0A; states 0x0A and 0x0B wait on the gate
 *           halfword D_80143C40, where 0x0A runs func_8014ECAC mode 0x10,
 *           func_801BE1B0 mode 7 and func_801C601C mode 2 before arming 0x0B,
 *           and 0x0B latches progress 0x2A and arms 0x0C; state 0x0C, at
 *           progress 0x2D, stores its claim in the scratchpad cell 0x1F800000
 *           and, unless that claim reads 0xFF, seeds the selected row (+0 = 1,
 *           +5 = 0x13, +9 = 0x50, the signed word +0x64 = -0x32E, and +0x68 and
 *           +0x6C from the signed halfwords D_801492DA and D_801492DC) before
 *           latching D_80146867 to the claimed index and arming 0x0D; state
 *           0x0D stays while the row that index names is still occupied and
 *           otherwise claims again with the same field run (+0x64 = -0x2AA,
 *           +9 = 0x28) before arming 0x0E; state 0x0E, once that row is free,
 *           latches progress 0x2F, clears D_80146867 and arms 0x11; state 17
 *           at progress 0x35 runs func_8015D404(0, 2) and
 *           func_80161C20(0x5C, 0x64, 8) and arms 0x12; state 18 at progress
 *           0x42 runs func_8015D554(0, 1, 4, 0x1E) and func_801636A0(2, 1) and
 *           arms 0x13; state 19, once func_80163EA0 answers a nonzero low byte,
 *           queues the same 0x5C/0x64/8 cue, latches progress 0x43 and arms
 *           0x14; state 20 at progress 0x47 walks the selection array
 *           0x80144F5A three times, queueing action 0x25 for each slot holding
 *           2, 0x24 for each slot holding 5 and 0x23 for each slot holding 1,
 *           then latches the phase byte to 2 and arms 0x15; state 21, unless
 *           the phase byte reads 2, latches progress 0x48 and arms 0x16; state
 *           22 at progress 0x49 publishes D_8014686C through func_8015B580
 *           mode 0x2C, runs func_801650B4(0, 0x57, 1, 0) and joins the reset
 *           tail; state 30 queues action 0x26, latches the phase byte to 2 and
 *           arms secondary state 1. States 1 and 22 join one reset tail that
 *           clears D_80146865 to D_80146867, runs the shared overlay release
 *           helper func_8015C058 and clears both scene state bytes. The
 *           0x20-byte frame keeps $ra, $s1 and $s0 across the calls, the
 *           state-byte load and its bound test are hoisted above the frame
 *           setup, and the 31-word case table sits in the payload leading table
 *           data at 0x801F7174.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState15Phases(
    void) {
  switch (D_80146875) {
    case 0:
      func_80150224(0x1A);
      D_80143BB0 = 2u;
      D_80146875 = 1u;
      break;

    case 1:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 5:
      func_80150224(0x1B);
      D_80143BB0 = 2u;
      D_80146875 = 6u;
      break;

    case 6: {
      u8 index;
      u8* record;

      if (D_80143BB0 == 2u) {
        break;
      }

      index = func_8019601C();

      if (index != 0xFFu) {
        record = (u8*)D_80143FC8 + (u32)index * 0x74;
        record[0] = 1u;
        record[5] = 0x4Bu;
      }

      func_801BE1B0(6u);
      D_8014933B = 1u;
      func_8015B580((u32)&D_80144F28, 7);
      func_8015DF18(0x207);
      func_8015DF18(0x201);
      D_80146875 = 9u;
      break;
    }

    case 9:
      if (g_ScenarioProgress != 0x29u) {
        break;
      }

      func_8014ECAC(0x0F);
      D_80146875 = 0xAu;
      break;

    case 10:
      if (D_80143C40 != 0u) {
        break;
      }

      func_8014ECAC(0x10);
      func_801BE1B0(7u);
      func_801C601C(2u);
      D_80146875 = 0xBu;
      break;

    case 11:
      if (D_80143C40 != 0u) {
        break;
      }

      g_ScenarioProgress = 0x2Au;
      D_80146875 = 0xCu;
      break;

    case 12: {
      u8* scratch;
      u8 object_index;

      if (g_ScenarioProgress != 0x2Du) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x32E;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x50u;
        D_80146875 = 0xDu;
        D_80146867 = *scratch;
      }
      break;
    }

    case 13: {
      u8* slot = &D_80146867;
      u8* scratch;
      u8 object_index;
      u32 row;

      row = (u32)*slot * 0x74u;

      if (D_80143FC8[row] != 0u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x28u;
        D_80146875 = 0xEu;
        *slot = *scratch;
      }
      break;
    }

    case 14: {
      u8* slot = &D_80146867;

      if (D_80143FC8[(u32)*slot * 0x74u] != 0u) {
        break;
      }

      g_ScenarioProgress = 0x2Fu;
      *slot = 0u;
      D_80146875 = 0x11u;
      break;
    }

    case 17:
      if (g_ScenarioProgress != 0x35u) {
        break;
      }

      func_8015D404(0, 2);
      func_80161C20(0x5C, 0x64, 8);
      D_80146875 = 0x12u;
      break;

    case 18:
      if (g_ScenarioProgress != 0x42u) {
        break;
      }

      func_8015D554(0, 1, 4, 0x1E);
      func_801636A0(2, 1);
      D_80146875 = 0x13u;
      break;

    case 19:
      if ((u8)func_80163EA0() == 0) {
        break;
      }

      func_80161C20(0x5C, 0x64, 8);
      g_ScenarioProgress = 0x43u;
      D_80146875 = 0x14u;
      break;

    case 20: {
      u8 i;

      if (g_ScenarioProgress != 0x47u) {
        break;
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 2u) {
          func_80150224(0x25u);
        }
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 5u) {
          func_80150224(0x24u);
        }
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 1u) {
          func_80150224(0x23u);
        }
      }

      D_80143BB0 = 2u;
      D_80146875 = 0x15u;
      break;
    }

    case 21:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0x48u;
      D_80146875 = 0x16u;
      break;

    case 22:
      if (g_ScenarioProgress != 0x49u) {
        break;
      }

      func_8015B580(D_8014686C, 0x2C);
      func_801650B4(0, 0x57, 1, 0);
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 30:
      func_80150224(0x26);
      D_80143BB0 = 2u;
      D_80146875 = 1u;
      break;
  }
}
