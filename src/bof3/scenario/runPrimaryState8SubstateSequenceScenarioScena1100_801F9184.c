#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_1F800000;
extern u8 D_80143BB0;
extern u16 D_80143C40;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;
/* The 0x74-byte row at D_80143FC8 is seeded through one extern base per field
 * because the original re-materialises `lui at,0x8014` for every statement: a
 * single struct base lets the compiler fold the trailing stores into one
 * base+offset chain. Same spelling as the exact scena00 and scena16 row
 * seeders. */
extern u8 D_80143FC8[];
extern u8 D_80143FC9[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_80143FD4[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern s16 D_801492DA;
extern u32 D_80149308;
extern u32 D_8014930C;
extern u16 D_8014932C;
extern u16 D_8014932E;
extern u8 D_80149332;
extern u8 D_80149333;
extern u8 D_8014933B;

/* Shared front-end action starter at 0x80150224. */
void func_80150224(u32 arg0);
/* Shared screen-position request at 0x8015507C. */
void func_8015507C(s16 x, s16 y);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end record allocator at 0x8019601C; answers 0xFF when none is
 * free. */
u8 func_8019601C(void);
/* Shared front selection/context request at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared effect-group helper at 0x801C601C. */
void func_801C601C(s32 arg0);
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F9184
 * @behavior Word 8 (the word stored at 0x801FAF1C) of the ten-entry
 * overlay-local second-level handler run at 0x801FAEFC that the exact
 * dispatcher dispatchSecondaryStateScenarioScena1100_801F7748 (0x801F7748)
 * indexes with the shared primary state byte D_80146874, so a primary state
 * reading 8 selects it, and no jal or j in the payload reaches the address. It
 * switches on the shared secondary sub-state byte D_80146875 through the
 * forty-word absolute jump table jtbl_801F6FEC_main at 0x801F6FEC in this
 * payload's leading table data, and the table words of sub-states 5, 23
 * through 26, 28, 30 and 32 are the ones that point at the body exit.
 * Sub-state 0, unconditionally, stores 1 in the sub-state byte, sets flag bit
 * 0xE of the shared flag word D_8014686C through func_8015B580 and requests
 * the front selection context (0x65, 0x240000, 0x250000, 0x80) through
 * func_8019FA28.
 * Sub-states 1, 3, 6, 8, 14, 18, 20, 21 each claim a front-end record through
 * the allocator func_8019601C, publish the index in the scratchpad byte
 * D_1F800000 and, unless that index reads 0xFF, seed the 0x74-byte row of
 * D_80143FC8 it selects: +0 = 1, +5 = 0x13 (0x31 on sub-states 3, 18 and 21;
 * 0x18 on the second claim of sub-state 14), the signed word at +0x64 = the
 * sub-state's own literal, the word at +0x68 = the signed halfword
 * D_801492DA, the word at +0x6C, the byte at +9 and, on sub-states 3, 18 and
 * 21, the word at +0x0C. Sub-state 1 also runs func_801C601C(0), stores 2 in
 * D_80149333 and lowers the halfword D_8014932C by 0x500; sub-state 3 also
 * stores 0x220000 in D_80149308, 0x230000 in D_8014930C and 4 in the halfword
 * D_8014932E; sub-state 14 also dispatches cue 0x200, runs func_801C601C(1),
 * stores 2 in D_80149333, lowers D_8014932C by 0x780, and after its first claim
 * requests the screen position func_8015507C(0x53, 0x1B) and claims a second
 * record, then stores 0x23 in D_8014933B. Sub-states 6, 14, 18, 20 and 21
 * republish the index they claimed into D_80146867, sub-state 14 after its
 * first claim, while sub-states 1, 3 and 8 advance with only their sub-state
 * byte.
 * Sub-state 4 waits for the byte D_80149332 to leave zero, then stores 1 in the
 * progress byte g_ScenarioProgress and advances to 6. Sub-states 7, 15, 19, 21
 * and 22 test the byte +0 of the row D_80146867 selects before advancing.
 * The remaining arms gate on the shared progress byte g_ScenarioProgress: 6 on
 * 4, 8 on 0x0A, 9 and 13 on 0x14 (13 through an address-taken pointer), 18 and
 * 35 on 2, 20 on 4, 21 on 6, 27 on 0x16, 29 on 1, 31 on 3, 33 on 0x0A and 39
 * on 0x15; sub-states 10, 17 and 36 wait for the gate halfword D_80143C40 to be
 * clear and sub-states 12, 16 and 37 for the front-end mode byte D_80143BB0 to
 * leave 2. The arms also use the flag words D_80146865, D_80146866 and
 * D_8014832E, and sub-state 39 runs func_8015B580(D_8014686C, 0x12) and the
 * overlay release helper func_8015C058 before clearing the progress byte,
 * D_80146865, D_80146866, D_80146867, the sub-state byte and the primary state
 * byte, which ends the chain.
 * Takes no arguments and returns nothing; its 0x20-byte frame keeps $ra, $s1
 * and $s0 across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState8SubstateSequenceScenarioScena1100_801F9184(void) {
  switch (D_80146875) {
    case 0:
      D_80146875 = 1u;
      func_8015B580(D_8014686C, 0xE);
      func_8019FA28(0x65, 0x240000, 0x250000, 0x80);
      break;

    case 1: {
      u8* scratch;
      u8  object_index;

      func_801C601C(0);
      D_80149333 = 2u;
      D_8014932C = (u16)(D_8014932C - 0x500u);

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x1a2;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
        D_80143FD1[(u32)*scratch * 0x74u] = 4u;
        D_80146875 = 2u;
      }
      break;
    }

    case 2:
      D_8014832E = 0x1fu;
      D_80146875 = 3u;
      break;

    case 3: {
      u8* scratch;
      u8  object_index;

      D_80149308 = 0x220000u;
      D_8014930C = 0x230000u;
      D_8014932E = 4u;

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x31u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2aa;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
        *(s32*)(D_80143FD4 + (u32)*scratch * 0x74u) = 0;
        D_80143FD1[(u32)*scratch * 0x74u] = 0xffu;
        D_80146875 = 4u;
      }
      break;
    }

    case 4:
      if (D_80149332 != 0u) {
        break;
      }
      g_ScenarioProgress = 1u;
      D_80146875 = 6u;
      break;

    case 6: {
      u8* scratch;
      u8  object_index;

      if (g_ScenarioProgress != 4u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2ec;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x0au;
        D_80146875 = 7u;
        D_80146867 = *scratch;
      }
      break;
    }

    case 7: {
      u8* claimed;

      claimed = &D_80146867;
      if (D_80143FC8[(u32)*claimed * 0x74u] == 0u) {
        g_ScenarioProgress = 5u;
        *claimed = 0u;
        D_80146875 = 8u;
      }
    }
      /* fall through */
    case 8: {
      u8* scratch;
      u8  object_index;

      if (g_ScenarioProgress != 0x0au) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2aa;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x0au;
        D_80146875 = 9u;
      }
      break;
    }

    case 9:
      if (g_ScenarioProgress != 0x14u) {
        break;
      }
      func_8014ECAC(0xD);
      D_80146875 = 0x0au;
      break;

    case 10:
      if (D_80143C40 != 0u) {
        break;
      }
      D_8014832E = 0u;
      D_80146875 = 0x0bu;
      break;

    case 11:
      func_80150224(4u);
      D_80143BB0 = 2u;
      D_80146875 = 0x0cu;
      break;

    case 12:
      if (D_80143BB0 == 2u) {
        break;
      }
      D_80146875 = 0x0du;
      g_ScenarioProgress = 0u;
      D_80146866 = 1u;
      func_8019FA28(0x65, 0x490000, 0x90000, 0x81);
      break;

    case 13: {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 0x14u) {
        break;
      }
      *progress = 0u;
      D_80146866 = 1u;
      D_80146875 = 0x0eu;
      func_8019FA28(0x84, 0x530000, 0x1b0000, 0x83);
      break;
    }

    case 14: {
      u8* scratch;
      u8  object_index;

      dispatchSoundCue(0x200);
      func_801C601C(1);
      D_80149333 = 2u;
      D_8014932C = (u16)(D_8014932C - 0x780u);

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x3c8;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x258;
        D_80143FD1[(u32)*scratch * 0x74u] = 4u;
        D_80146875 = 0x0fu;
        D_80146867 = *scratch;
      }

      func_8015507C(0x53, 0x1b);

      object_index = func_8019601C();
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x18u;
        D_80143FC9[(u32)*scratch * 0x74u] = 0x5fu;
      }

      D_8014933B = 0x23u;
      break;
    }

    case 15: {
      u8* claimed;

      claimed = &D_80146867;
      if (D_80143FC8[(u32)*claimed * 0x74u] != 0u) {
        break;
      }
      *claimed = 0u;
      func_80150224(0x1au);
      D_80143BB0 = 2u;
      D_80146875 = 0x10u;
      break;
    }

    case 16:
      if (D_80143BB0 == 2u) {
        break;
      }
      func_8014ECAC(0xD);
      D_80146875 = 0x11u;
      break;

    case 17:
      if (D_80143C40 != 0u) {
        break;
      }
      g_ScenarioProgress = 1u;
      D_8014832E = 0x1fu;
      D_80146875 = 0x12u;
      break;

    case 18: {
      u8* scratch;
      u8  object_index;

      if (g_ScenarioProgress != 2u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x31u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x370;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x258;
        *(s32*)(D_80143FD4 + (u32)*scratch * 0x74u) = 0x580;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x3cu;
        D_80146875 = 0x13u;
        D_80146867 = *scratch;
      }
      break;
    }

    case 19: {
      u8* claimed;

      claimed = &D_80146867;
      if (D_80143FC8[(u32)*claimed * 0x74u] != 0u) {
        break;
      }
      g_ScenarioProgress = 3u;
      *claimed = 0u;
      D_80146875 = 0x14u;
      break;
    }

    case 20: {
      u8* scratch;
      u8  object_index;

      if (g_ScenarioProgress != 4u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x256;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0xe2;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x3cu;
        D_80146875 = 0x15u;
        D_80146867 = *scratch;
      }
      break;
    }

    case 21: {
      u8* scratch;
      u8  object_index;
      u32 row;

      if (g_ScenarioProgress != 6u) {
        break;
      }
      row = (u32)D_80146867 * 0x74u;
      if (D_80143FC8[row] != 0u) {
        break;
      }
      D_80146867 = 0u;

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x31u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x386;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0xe2;
        *(s32*)(D_80143FD4 + (u32)*scratch * 0x74u) = 0;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x3cu;
        D_80146875 = 0x16u;
        D_80146867 = *scratch;
      }
      break;
    }

    case 22: {
      u8* claimed;

      claimed = &D_80146867;
      if (D_80143FC8[(u32)*claimed * 0x74u] != 0u) {
        break;
      }
      g_ScenarioProgress = 7u;
      *claimed = 0u;
      D_80146875 = 0x1bu;
      break;
    }

    case 27: {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 0x16u) {
        break;
      }
      *progress = 0u;
      D_80146866 = 2u;
      D_80146865 = 0u;
      D_8014832E = 0u;
      D_80146875 = 0x1du;
      func_8015B580(D_8014686C, 0xF);
      dispatchSoundCue(0x201);
      func_8019FA28(0x84, 0x50000, 0x460000, 0x84);
      break;
    }

    case 29: {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 1u) {
        break;
      }
      *progress = 2u;
      D_80146875 = 0x1fu;
      break;
    }

    case 31: {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 3u) {
        break;
      }
      *progress = 4u;
      D_80146875 = 0x21u;
      break;
    }

    case 33: {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 0x0au) {
        break;
      }
      *progress = 0u;
      D_80146875 = 0x22u;
      func_8015B580(D_8014686C, 0x10);
      func_8019FA28(0x83, 0x200000, 0x2f0000, 0x87);
      break;
    }

    case 34:
      func_801C601C(1);
      D_80146875 = 0x23u;
      break;

    case 35:
      if (g_ScenarioProgress != 2u) {
        break;
      }
      func_8014ECAC(4);
      D_80146875 = 0x24u;
      break;

    case 36:
      if (D_80143C40 != 0u) {
        break;
      }
      D_80146866 = 2u;
      g_ScenarioProgress = 0u;
      D_8014832E = 0u;
      D_80146875 = 0x25u;
      break;

    case 37:
      if (D_80143BB0 == 2u) {
        break;
      }
      D_80146875 = 0x26u;
      func_8015B580(D_8014686C, 0x11);
      func_8019FA28(0x83, 0x50000, 0x5d0000, 0x88);
      break;

    case 38:
      func_801C601C(2);
      D_80146875 = 0x27u;
      break;

    case 39: {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 0x15u) {
        break;
      }
      func_8015B580(D_8014686C, 0x12);
      func_8015C058();
      *progress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
    }
  }
}
