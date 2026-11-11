#include "bof3/bof3.h"

/* Shared scenario progress byte of the main-executable data segment, owned by
 * the shared symbol map.
 * @source 0x80146864 @kind unknown
 */
extern u8 g_ScenarioProgress;

/* Signed primary-state byte of the shared state tuple; this overlay's
 * dispatchPrimaryStateHandlerScenarioScena0400_801F863C indexes the handler run
 * at 0x801FA210 with it.
 * @source 0x80146874 @kind unknown
 */
extern s8 D_80146874;

/* Substate byte of the same shared tuple. This body is the state controller
 * keyed on it.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

/* Shared countdown halfword of the same tuple, decremented by several substates
 * of this body and armed by others.
 * @source 0x80146876 @kind unknown
 */
extern u16 D_80146876;

/* Substate-independent cue selector byte read by substate 1.
 * @source 0x80145029 @kind unknown
 */
extern u8 D_80145029;

/* Mode word of the shared scenario block; substates 16 and 21 raise and clear
 * its bit 7 around the front-selector request.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Shared scenario flag word passed to func_8015B580 by substates 16 and 22.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

/* Effect-bank selector byte written 0x1F by substates 2, 5 and 8 and cleared by
 * substates 3 and 6.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;

/* Row byte 0 of the shared 0x74-byte record table at 0x80143FC8: the
 * occupancy flag the claim blocks of this body set to 1.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 5, the entry handler selector, seeded with 0x13 by the claim blocks
 * of this body and with 0x2C by substate 14.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 9, seeded with the per-state literal of each claim block.
 * @source 0x80143FD1 @kind unknown
 */
extern u8 D_80143FD1[];
/* Row word 0x64, the first seeded coordinate word.
 * @source 0x8014402C @kind unknown
 */
extern u8 D_8014402C[];
/* Row word 0x68, the second seeded coordinate word.
 * @source 0x80144030 @kind unknown
 */
extern u8 D_80144030[];
/* Row word 0x6C, the third seeded coordinate word.
 * @source 0x80144034 @kind unknown
 */
extern u8 D_80144034[];

/* Shared coordinate halfword triple the claim blocks of this body seed rows
 * from: substate 8 stores -0x35A in the first and every claim block reads them
 * as signed halfwords, so all three keep the signed width the original loads
 * with.
 * @source 0x801492D8 @kind unknown
 */
extern s16 D_801492D8;
/* @source 0x801492DA @kind unknown */
extern s16 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern s16 D_801492DC;

/* Shared render word pair written by substates 4 and 7.
 * @source 0x80149308 @kind unknown
 */
extern u32 D_80149308;
/* @source 0x8014930C @kind unknown */
extern s32 D_8014930C;

/* Shared 16-bit distance-to-target word that substates 9 and 16 step and clamp
 * and substates 8, 9 and 11 seed.
 * @source 0x8014932C @kind unknown
 */
extern u16 D_8014932C;

/* Shared status byte set to 1 by substate 2.
 * @source 0x8014933B @kind unknown
 */
extern u8 D_8014933B;

/* Shared flag bytes cleared by substates 16 and 21.
 * @source 0x80143F1D @kind unknown
 */
extern u8 D_80143F1D;
/* @source 0x80143F1E @kind unknown */
extern u8 D_80143F1E;

/* Scratchpad byte the shared object allocator func_8019601C answers through.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000;

void func_80153DE8(void);
void func_80154FD8(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void func_80161B24(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_80161BBC(u32 slot_id);
void func_801A78F8(void);
void func_801BE1B0(u32 arg0);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void addSignedTableStepToWord80149318ScenarioScena0400_801F9CFC(void);

/* @source 0x801F8F98
 * @behavior The overlay handler of primary state 2: word 2 of the primary-state
 *           handler run at 0x801FA210, which
 *           dispatchPrimaryStateHandlerScenarioScena0400_801F863C indexes with
 *           the signed shared state byte D_80146874. It switches on the shared
 *           substate byte D_80146875 with an unsigned bound of 31, so values
 *           0..30 select a case and 23..29 fall straight to the exit, and after
 *           the switch every substate from 2 to 12 steps the shared word
 *           D_80149318 through the overlay's signed step table by calling
 *           addSignedTableStepToWord80149318ScenarioScena0400_801F9CFC, which
 *           is what makes this body one per-frame substate controller.
 *           Substates 0, 3, 5, 11, 12 and 13 wait out the shared countdown
 *           D_80146876 and decrement it while it is non-zero; substates 9, 10,
 *           18, 19 and 21 decrement it first and test the result, and substate
 *           16 ticks D_8014932C up by 9 with a clamp at 0x400 on frames where
 *           the countdown is still non-zero. Expiry of the countdown loads this
 *           area's opening sequence: substate 0 clears the progress byte and
 *           calls func_801BE1B0(4); substate 3 clears the effect-bank byte
 *           D_8014832E; substate 5 arms that byte to 0x1F and increments
 *           D_80146864; substate 9 clears D_8014932C and arms the countdown to
 *           0x1E; substates 11 and 12 arm 0xF and 0x1E; substate 13 calls
 *           func_80154FD8(0x180); substate 8 arms 0x1F, seeds D_801492D8 with
 *           -0x35A, D_801492DC with 0x74 and D_8014932C with 0x400, calls
 *           func_80154FD8(0x100), increments D_80146864, requests cue
 *           func_80161B24(0x41, 0x64, 8), calls func_801BE1B0(5) and arms the
 *           countdown to 0x1E; substate 19 arms 0x14; substate 20 arms 0x1E;
 *           substate 21 arms 0x16. The progress gates run the requests the
 *           other way: substate 1 waits for D_80146864 == 0xC before
 *           func_80161CD0(D_80145029, 0x64, 8) and func_8015DF18(0x204);
 *           substate 2 waits for 0xF before setting D_8014933B = 1 and arming
 *           the countdown to 0x60; substate 6 waits for 0x11 before clearing
 *           D_8014832E; substate 14 waits for 0x22 and substate 15 for 0x23;
 *           substate 17 waits for 2 before func_8015D4F8(0, 1, 0x6E, 0x10);
 *           substate 20 waits for 3; substate 22 waits for 0x11 before
 *           releasing the front selector with func_8015C058 and
 *           func_8015B580(D_8014686C, 0xC), clearing D_80146864, calling
 *           func_801A78F8 and resetting both D_80146874 and D_80146875 to 0,
 *           which leaves primary state 2 and returns the overlay to its entry
 *           state. Substates 8, 10, 15, 18 and 19 claim one 0x74-byte row of the
 *           shared record table at 0x80143FC8 from the allocator
 *           func_8019601C: they store the returned byte into the scratchpad cell
 *           0x1F800000 and, unless that byte is 0xFF, set row byte 0 to 1 and
 *           seed row byte 5 (0x13, or 0x2C in substate 14), the two or three
 *           coordinate words at row 0x64/0x68/0x6C and row byte 9. Substates 4
 *           and 7 write the shared render word pair D_80149308/D_8014930C and
 *           reset the overlay through func_80153DE8, and substate 7 also passes
 *           0x41 to func_80161BBC before handing over to substate 30, which
 *           waits for the loader poll func_80162D00 and arms the countdown to 4.
 *           Substates 16 and 21 bracket the front-selector request
 *           func_8019FA28 with bit 7 of D_80146258: substate 16 raises it,
 *           requests context (0x41, 0x2E0000, 0x210000, 1), sets D_80143F1D to
 *           0xFF and calls func_8015B580(D_8014686C, 7); substate 21 clears it,
 *           requests context (0x36, 0xE0000, 0x100000, 0x80) and clears
 *           D_80143F1E. The body takes no arguments, returns nothing, and its
 *           0x20-byte frame holds $ra over $s1 and $s0.
 *
 * Shape notes from the original bytes: the dispatch is a 31-entry table the
 * original placed in the payload's leading table data at 0x801F6C84, not beside
 * the code, so this target's manifest carries the
 * matching.section_placements row that moves the compiler's .rodata table onto
 * those 0x7C bytes; with it the emitted table words compare equal slot for slot,
 * including the seven default slots of substates 23..29 that no case owns. The
 * case arms are written in the order the original lays them out, which puts
 * substate 30 between substates 7 and 8. Every cell this body touches twice in
 * one arm is reached through a local pointer, because the original holds those
 * addresses in registers - $v1 or $a0 where the arm ends in the shared tail, $s0
 * where the address lives across calls - and reads the bytes back through the
 * pointer; the single-read arm guards keep the direct symbol form, which is the
 * folded lui/load pair the original emits there. The countdown decrement of
 * substates 0, 3, 5, 11, 12 and 13 and the progress increment of substates 5, 9,
 * 11 and 12 are one shared tail each, so every one of those arms jumps into
 * those blocks. Substate 8's branch to its common path lands on the countdown
 * store rather than on the record-seed block, so the 0x1E store and the substate
 * store sit outside the claim condition, and substate 21 re-reads the countdown
 * through the pointer for its zero test instead of reusing the decremented
 * value. A payload scan of every EMI overlay found no byte-identical twin of
 * this 0xD64 span: the nearest siblings, scena01/00 at payload offset 0x2EA0,
 * scena02/00 at 0x476C, scena06/00 at 0x470C, 0x5188 and 0x5898, scena07/00 at
 * 0x64DC, scena09/00 at 0x35F0, scena11/00 at 0x1308/0x17C8/0x2584 and
 * scena12/00 at 0x4C64, share only the dispatch prologue and differ in more
 * than 800 of the span's 857 words, so the clone lever was declined and this
 * body was seeded from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState2SubstateSequenceScenarioScena0400_801F8F98(void) {
  switch (D_80146875) {
    case 0: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      g_ScenarioProgress = 0;
      func_801BE1B0(4u);
      D_80146875 = 1u;
      break;
    }

    case 1:
      if (g_ScenarioProgress != 0xcu) {
        break;
      }

      func_80161CD0(D_80145029, 0x64, 8);
      func_8015DF18(0x204u);
      D_80146875 = 2u;
      break;

    case 2:
      if (g_ScenarioProgress != 0xfu) {
        break;
      }

      D_8014933B = 1u;
      D_80146876 = 0x60u;
      D_80146875 = 3u;
      break;

    case 3: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      D_8014832E = 0u;
      D_80146875 = 4u;
      break;
    }

    case 4:
      D_80149308 = 0x4e0000u;
      D_8014930C = 0x280000;
      func_80153DE8();
      D_80146876 = 4u;
      D_80146875 = 5u;
      break;

    case 5: {
      u16* counter = &D_80146876;
      u8*  progress;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      D_8014832E = 0x1fu;
      D_80146875 = 6u;
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      break;
    }

    case 6:
      if (g_ScenarioProgress != 0x11u) {
        break;
      }

      D_8014832E = 0u;
      D_80146875 = 7u;
      break;

    case 7:
      D_80149308 = 0x850000u;
      D_8014930C = 0xa0000;
      func_80153DE8();
      func_80161BBC(0x41u);
      D_80146875 = 0x1eu;
      break;

    case 30:
      if (func_80162D00() == 0) {
        break;
      }

      D_80146876 = 4u;
      D_80146875 = 8u;
      break;

    case 8: {
      u16* counter = &D_80146876;
      s16* position;
      u8*  progress;
      u8*  scratch;
      u8   object_index;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      D_8014832E = 0x1fu;
      position = &D_801492D8;
      *position = -0x35a;
      D_801492DC = 0x74u;
      D_8014932C = 0x400u;
      func_80154FD8(0x100u);
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      func_80161B24(0x41u, 0x64, 8);
      func_801BE1B0(5u);
      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = *position;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x192;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x3cu;
      }

      *counter = 0x1eu;
      D_80146875 = 9u;
      break;
    }

    case 9: {
      u16* counter = &D_80146876;
      u8*  progress;
      u16  count;
      s32  distance;

      count = *counter - 1;
      *counter = count;

      if (count != 0) {
        distance = D_8014932C - 0x22;
        D_8014932C = distance;

        if ((s16)distance < 0) {
          D_8014932C = 0u;
        }
        break;
      }

      D_8014932C = 0u;
      *counter = 0x1eu;
      D_80146875 = 0xau;
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      break;
    }

    case 10: {
      u16* counter = &D_80146876;
      u16  count;
      u8*  scratch;
      u8   object_index;

      count = *counter - 1;
      *counter = count;

      if (count != 0) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2d6;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x2b0;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x3cu;
      }

      *counter = 0xfu;
      D_80146875 = 0xbu;
      break;
    }

    case 11: {
      u16* counter = &D_80146876;
      u8*  progress;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      *counter = 0xfu;
      D_80146875 = 0xcu;
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      break;
    }

    case 12: {
      u16* counter = &D_80146876;
      u8*  progress;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      *counter = 0x1eu;
      D_80146875 = 0xdu;
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      break;
    }

    case 13: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      func_80154FD8(0x180u);
      D_80146875 = 0xeu;
      break;
    }

    case 14: {
      u8* scratch;
      u8  object_index;

      if (g_ScenarioProgress != 0x22u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index == 0xffu) {
        break;
      }

      D_80143FC8[(u32)object_index * 0x74u] = 1u;
      D_80143FCD[(u32)*scratch * 0x74u] = 0x2cu;
      D_80146875 = 0xfu;
      break;
    }

    case 15: {
      u8* scratch;
      u8  object_index;

      if (g_ScenarioProgress != 0x23u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x3b2;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x1eu;
      }

      D_80146876 = 0xa0u;
      D_80146875 = 0x10u;
      break;
    }

    case 16: {
      u16* counter = &D_80146876;
      u16  count;
      s32  distance;

      count = *counter;

      if (count != 0) {
        distance = D_8014932C + 9;
        D_8014932C = distance;

        if (0x400 < (s16)distance) {
          D_8014932C = 0x400u;
        }

        *counter = count - 1;
        break;
      }

      D_80146258 = D_80146258 | 0x80u;
      func_8019FA28(0x41u, 0x2e0000u, 0x210000u, 1u);
      D_80143F1D = 0xffu;
      func_8015B580(D_8014686C, 7);
      D_80146875 = 0x11u;
      break;
    }

    case 17:
      if (g_ScenarioProgress != 2u) {
        break;
      }

      func_8015D4F8(0u, 1u, 0x6e, 0x10);
      D_80146876 = 0x1fu;
      D_80146875 = 0x12u;
      break;

    case 18: {
      u16* counter = &D_80146876;
      u16  count;
      u8*  scratch;
      u8   object_index;

      count = *counter - 1;
      *counter = count;

      if (count != 0xcu) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x58;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0xau;
      }

      D_80146875 = 0x13u;
      break;
    }

    case 19: {
      u16* counter = &D_80146876;
      u16  count;
      u8*  scratch;
      u8   object_index;

      count = *counter - 1;
      *counter = count;

      if (count != 0) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = D_801492D8;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = -0x262;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x20u;
      }

      D_80146875 = 0x14u;
      break;
    }

    case 20:
      if (g_ScenarioProgress != 3u) {
        break;
      }

      D_80146876 = 0x1eu;
      D_80146875 = 0x15u;
      break;

    case 21: {
      u16* counter = &D_80146876;
      u16  count;

      count = *counter - 1;
      *counter = count;

      if (0x13u <= count) {
        addSignedTableStepToWord80149318ScenarioScena0400_801F9CFC();
      }

      if (*counter != 0) {
        break;
      }

      D_80146258 = D_80146258 & 0xff7fu;
      func_8019FA28(0x36u, 0xe0000u, 0x100000u, 0x80u);
      D_80143F1E = 0u;
      D_80146875 = 0x16u;
      break;
    }

    case 22: {
      u8* progress = &g_ScenarioProgress;

      if (*progress != 0x11u) {
        break;
      }

      func_8015C058();
      *progress = 0;
      func_8015B580(D_8014686C, 0xc);
      func_801A78F8();
      D_80146874 = 0;
      D_80146875 = 0u;
      break;
    }
  }

  if ((u32)(D_80146875 - 2) < 11u) {
    addSignedTableStepToWord80149318ScenarioScena0400_801F9CFC();
  }
}
