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
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* The shared three-byte front-end selection array: the six scan arms of this
 * body walk it as a table and the claim arms read its head at 0x80144F5A.
 * @source 0x80144F5A @kind unknown
 */
extern u8 D_80144F5A[];
/* Front-end record table and the field bases of its 0x74-byte rows: the
 * original re-materialises `lui at,0x8014` for every store, so each field is
 * addressed as its own absolute symbol rather than through one struct base.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FCE @kind unknown */
extern u8 D_80143FCE[];
/* @source 0x80143FD4 @kind unknown */
extern u8 D_80143FD4[];
/* @source 0x80143FD8 @kind unknown */
extern u8 D_80143FD8[];
/* @source 0x80143FF6 @kind unknown */
extern u8 D_80143FF6[];
/* @source 0x80143FF8 @kind unknown */
extern u8 D_80143FF8[];
/* The three shared work records the claim arms publish, one per selection
 * slot, 0x140 bytes apart.
 * @source 0x80145E90 @kind unknown
 */
extern u8 D_80145E90[];
/* @source 0x80145FD0 @kind unknown */
extern u8 D_80145FD0[];
/* @source 0x80146110 @kind unknown */
extern u8 D_80146110[];
/* Scratchpad work-object pointer cell; the original re-materialises its
 * address for every access, so it keeps the symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Overlay-local byte table at the head of this target's table data; the index
 * is the selection array's head byte.
 * @source 0x801FE3BC @kind table
 */
extern u8 D_801FE3BC[];

void func_80150224(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015D404(u32 arg0, s32 arg1);
void func_801BE1B0(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C601C(u32 arg0);
void func_801C7A54(s32 arg0);
/* Sibling subsegment of this same overlay that seeds the front-end record
 * table from the shared selection array. */
void func_801FBC88(void);
u8 func_8019601C(void);
void publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80(u8 step);

/* @source 0x801FB30C
 * @behavior Primary state 14 of this overlay, reached as word 14 of the
 *           34-word overlay handler table at 0x801FE3E4: it steps the shared
 *           secondary scene state byte D_80146875 through a 36-entry case
 *           table in which only states 0, 1, 2, 5, 10, 11, 12, 13, 20, 21, 22,
 *           23, 30 and 35 have a body; every other value, and every value
 *           above 0x23, falls to the epilogue. State 0 releases the overlay
 *           with mode 3 through func_801BE1B0, runs func_801C601C mode 0 and
 *           arms secondary state 1. State 1, at progress 4, walks the three
 *           selection slots once per looked-for value and queues cue 0x12 for
 *           each slot holding 2, cue 0x11 for each slot holding 5 and cue 0x10
 *           for each slot holding 1, then latches the shared phase byte
 *           D_80143BB0 to 2 and arms secondary state 2. State 2, unless the
 *           phase byte already reads 2, publishes each of the three shared
 *           work records in turn - D_80145E90, D_80145FD0 and D_80146110 - as
 *           the scratchpad cursor at 0x1F800044, but only for a selection slot
 *           that still reads zero, claims one front-end record per published
 *           work record through func_8019601C and, unless the returned index
 *           is 0xFF, seeds the 0x74-byte row at 0x80143FC8 that the index
 *           selects: record byte 0x00 = 1, byte 0x05 = 6, byte 0x06 = 3, word
 *           0x0C = 0, word 0x10 = the byte D_801FE3BC[head of the selection
 *           array] and the halfwords at 0x2E/0x30 copied from the work record
 *           at the same offsets; it then stores 5 into both D_80146875 and
 *           g_ScenarioProgress. States 5 and 13 release the overlay through
 *           func_8015B580 with flag bits 0x28 and 0x29 at progress 0x0A and
 *           0x0F, and state 35 clears bit 3 of the mode halfword D_80146258 and
 *           releases with flag bit 0x2A at progress 0x1E; those three arms join
 *           one reset tail that clears D_80146865..D_80146867, runs the shared
 *           overlay release helper func_8015C058 and clears both scene state
 *           bytes. State 10, unless the phase byte reads 2, seeds a front-end
 *           record through publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80
 *           with selector 0x93 and arms secondary state 0x0B; state 11 runs the
 *           sibling subsegment func_801FBC88; state 12, unless the phase byte
 *           reads 2, releases the overlay with mode 4 and arms state 0x0D;
 *           state 20 sets bit 3 of D_80146258, runs func_8015D404 with mode 0
 *           and releases the overlay with mode 5 before arming state 0x15;
 *           state 21, at progress 0x15, runs func_801C601C mode 1 and arms
 *           0x16; state 22, at progress 0x1A, runs func_801BFE34 with 0x2E0000,
 *           0x60000 and 0x1C and arms 0x17; state 23, at progress 0x1C, queues
 *           cue 0x19 for each slot holding 5, cue 0x18 for each slot holding 1
 *           and cue 0x17 for each slot holding 2, latches the phase byte to 2
 *           and arms state 0x1E; state 30, unless the phase byte reads 2, runs
 *           func_801C7A54 with 0x1C and arms state 0x23. Takes no arguments and
 *           returns nothing, and the 0x20-byte frame keeps $ra, $s1 and $s0
 *           across the calls; the state-byte load and its bound test are
 *           hoisted above the frame setup, and the 36-word case table sits in
 *           the overlay table data at 0x801F70E4.
 *
 * Evidence: the payload span 0x801FB30C-0x801FBC87 is one complete function.
 * Its prologue is the hoisted `lui/lbu` of D_80146875 with
 * `addiu $sp,-0x20` and the three saved-register stores behind it, its bound
 * test `sltiu 0x24` feeds the table dispatch, and its one epilogue is
 * `lw $ra,0x18($sp); lw $s1; lw $s0; addiu $sp,$sp,0x20; jr $ra; nop` at
 * 0x801FBC70. The next declared boundary 0x801FBC88 opens a different function
 * with its own frame, so the span is not a fragment. The 36 words at
 * 0x801F70E4 read from these original bytes name the fourteen case bodies and
 * twenty-two epilogue slots.
 *
 * Clone lever declined: a scan of the 398 extracted payloads under
 * out/binaries for this span's own prologue and dispatch words found the bytes
 * only in this payload at offset 0x470C, so no byte-identical twin exists. The
 * exact siblings runPrimaryState11Phases,
 * runPrimaryState16Phases and
 * runPrimaryState17Phases carry a
 * different case set each, so they are shape evidence only.
 *
 * Shape evidence: the claim-and-seed statement run is the exact sibling
 * publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80 of this same
 * target, transcribed three times with this span's own work record and
 * selection slot per copy and with the selector constant 3 written directly
 * instead of that sibling's caller value plus 0x70. The dispatch, the scan
 * loops and the reset tail follow the exact siblings above; this span's own
 * literals, guards and arm order come from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState14Phases(void) {
  switch (D_80146875) {
    case 0:
      func_801BE1B0(3u);
      func_801C601C(0u);
      D_80146875 = 1u;
      break;

    case 1: {
      u8 i;

      if (g_ScenarioProgress != 4u) {
        break;
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 2u) {
          func_80150224(0x12u);
        }
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 5u) {
          func_80150224(0x11u);
        }
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 1u) {
          func_80150224(0x10u);
        }
      }

      D_80143BB0 = 2u;
      D_80146875 = 2u;
      break;
    }

    case 2: {
      u8* work;

      if (D_80143BB0 == 2u) {
        break;
      }

      if (D_80144F5A[0] == 0u) {
        D_1F800044 = D_80145E90;
        D_1F800044[0x0B] = func_8019601C();

        if (D_1F800044[0x0B] != 0xFFu) {
          D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
          D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 6u;
          D_80143FCE[(u32)D_1F800044[0x0B] * 0x74u] = 3u;
          work = D_1F800044;
          *(u32*)(D_80143FD4 + (u32)work[0x0B] * 0x74u) = 0u;
          *(u32*)(D_80143FD8 + (u32)work[0x0B] * 0x74u) =
              D_801FE3BC[D_80144F5A[0]];
          *(u16*)(D_80143FF6 + (u32)work[0x0B] * 0x74u) =
              *(u16*)(work + 0x2E);
          *(u16*)(D_80143FF8 + (u32)work[0x0B] * 0x74u) =
              *(u16*)(work + 0x30);
        }
      }

      if (D_80144F5A[1] == 0u) {
        D_1F800044 = D_80145FD0;
        D_1F800044[0x0B] = func_8019601C();

        if (D_1F800044[0x0B] != 0xFFu) {
          D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
          D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 6u;
          D_80143FCE[(u32)D_1F800044[0x0B] * 0x74u] = 3u;
          work = D_1F800044;
          *(u32*)(D_80143FD4 + (u32)work[0x0B] * 0x74u) = 0u;
          *(u32*)(D_80143FD8 + (u32)work[0x0B] * 0x74u) =
              D_801FE3BC[D_80144F5A[1]];
          *(u16*)(D_80143FF6 + (u32)work[0x0B] * 0x74u) =
              *(u16*)(work + 0x2E);
          *(u16*)(D_80143FF8 + (u32)work[0x0B] * 0x74u) =
              *(u16*)(work + 0x30);
        }
      }

      if (D_80144F5A[2] == 0u) {
        D_1F800044 = D_80146110;
        D_1F800044[0x0B] = func_8019601C();

        if (D_1F800044[0x0B] != 0xFFu) {
          D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
          D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 6u;
          D_80143FCE[(u32)D_1F800044[0x0B] * 0x74u] = 3u;
          work = D_1F800044;
          *(u32*)(D_80143FD4 + (u32)work[0x0B] * 0x74u) = 0u;
          *(u32*)(D_80143FD8 + (u32)work[0x0B] * 0x74u) =
              D_801FE3BC[D_80144F5A[2]];
          *(u16*)(D_80143FF6 + (u32)work[0x0B] * 0x74u) =
              *(u16*)(work + 0x2E);
          *(u16*)(D_80143FF8 + (u32)work[0x0B] * 0x74u) =
              *(u16*)(work + 0x30);
        }
      }

      D_80146875 = 5u;
      g_ScenarioProgress = 5u;
      break;
    }

    case 5:
      if (g_ScenarioProgress != 0x0Au) {
        break;
      }

      func_8015B580(D_8014686C, 0x28);
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 10:
      if (D_80143BB0 == 2u) {
        break;
      }

      publishWorkBaseAndSeedFrontEndRecordScenarioScena0600_801FCB80(0x93);
      D_80146875 = 0xBu;
      break;

    case 11:
      func_801FBC88();
      break;

    case 12:
      if (D_80143BB0 == 2u) {
        break;
      }

      func_801BE1B0(4);
      D_80146875 = 0xDu;
      break;

    case 13:
      if (g_ScenarioProgress != 0x0Fu) {
        break;
      }

      func_8015B580(D_8014686C, 0x29);
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 20:
      D_80146258 |= 8u;
      func_8015D404(0u, 0);
      func_801BE1B0(5);
      D_80146875 = 0x15u;
      break;

    case 21:
      if (g_ScenarioProgress != 0x15u) {
        break;
      }

      func_801C601C(1);
      D_80146875 = 0x16u;
      break;

    case 22:
      if (g_ScenarioProgress != 0x1Au) {
        break;
      }

      func_801BFE34(0x2E0000u, 0x60000u, 0x1C);
      D_80146875 = 0x17u;
      break;

    case 23: {
      u8 i;

      if (g_ScenarioProgress != 0x1Cu) {
        break;
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 5u) {
          func_80150224(0x19u);
        }
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 1u) {
          func_80150224(0x18u);
        }
      }

      for (i = 0; i < 3u; i++) {
        if (D_80144F5A[i] == 2u) {
          func_80150224(0x17u);
        }
      }

      D_80143BB0 = 2u;
      D_80146875 = 0x1Eu;
      break;
    }

    case 30:
      if (D_80143BB0 == 2u) {
        break;
      }

      func_801C7A54(0x1C);
      D_80146875 = 0x23u;
      break;

    case 35:
      if (g_ScenarioProgress != 0x1Eu) {
        break;
      }

      D_80146258 ^= 8u;
      func_8015B580(D_8014686C, 0x2A);
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
  }
}
