#include "bof3/bof3.h"

extern u8 D_1F800000;
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern u8 D_80145029;
extern u16 D_80146258;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;
extern s16 D_801492D8;
extern s16 D_801492DA;
extern s16 D_801492DC;
extern u8 D_80181EBA[];
extern u8 D_80181EBB[];
extern u8 D_801FEA6C; /* @source 0x801FEA6C @kind data */
extern u8 g_ScenarioProgress;

void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
s32 func_8019601C(void);
void func_801BE1B0(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);

/* @source 0x801FBCA4
 * @behavior Entry 6 of the overlay progress-handler table at D_801FE970 that
 * dispatchProgressHandlerScenarioScena0800_801FAB5C indexes with the signed
 * byte D_80146874, so no in-image jal targets this address and that table word
 * is its only reference in this payload. It switches the shared sub-state byte
 * D_80146875 through the nine-entry absolute jump table at 0x801F6CBC in this
 * payload leading table data, the last entry and every value outside it
 * returning without touching the tail. Sub-state 0 starts the front selection
 * effect through func_8015D4F8 with the two 0x4-stride table bytes of the
 * shared selection index D_80145029 (D_80181EBA and D_80181EBB), the levels
 * 0x64 and 0x20, then arms the shared countdown halfword D_80146876 with 0x20
 * and advances to 1. Sub-state 1 decrements that countdown and, when the value
 * it replaced was zero, queues the front-end cue func_80161CD0(D_80145029,
 * 0x64, 0x20), clears the shared scenario progress byte g_ScenarioProgress,
 * runs func_801BE1B0(0) and advances to 2. Sub-state 2, at progress byte 1,
 * asks func_8019601C for an object index, publishes it at D_801FEA6C and,
 * unless that byte reads 0xFF, seeds that object row of the 0x74-byte table at
 * D_80143FC8 (+0 = 1, +5 = 0x13, word -0x318 at +0x64, the signed halfword
 * D_801492DA at +0x68, byte 0x1E at +9 and the signed halfword D_801492DC at
 * +0x6C) before advancing to 3. Sub-state 3 returns while bit 0 of the published
 * row flag byte at D_80143FC8 + D_801FEA6C * 0x74 is set; otherwise it claims a
 * slot, publishes it at scratchpad 0x1F800000 and, unless that byte reads 0xFF,
 * seeds that row (+0 = 1, +5 = 0x5F, +1 = 0), then claims a second slot,
 * publishes it at D_801FEA6C and, unless it reads 0xFF, seeds that row (+0 = 1,
 * +5 = 0x13, the signed halfword D_801492D8 at +0x64, word 0x1C at +0x6C, byte
 * 0xB4 at +9 and the signed halfword D_801492DA at +0x68), and finally advances
 * to 4 and raises the progress byte. Sub-state 4 returns while that published row flag
 * bit is set; otherwise it claims a slot, publishes it at D_801FEA6C and,
 * unless it reads 0xFF, seeds that row (+0 = 1, +5 = 0x13, word -0x2AA at
 * +0x64, word 0x200 at +0x6C, word 0 at +0x68 and byte 0x5A at +9), then runs
 * func_801BFE34(0x430000, 0x380000, 0x21), raises the progress byte,
 * func_801BE1B0(1) and advances to 5. Sub-state 5 raises the progress byte and
 * advances to 6 while that published row flag bit is clear. Sub-state 6, at
 * progress byte 5, runs func_801C7A54(0x21) and advances to 7. Sub-state 7 does
 * nothing. Sub-state 8 clears the progress byte, releases the shared selector
 * through func_8015C058, sets scenario flag bit 3 through
 * func_8015B580(D_8014686C, 8), returns the overlay to handler 0 by clearing
 * D_80146874 and the sub-state byte and clears flag bit 3 of the shared mode
 * halfword D_80146258. It takes no arguments, returns nothing, and its
 * 0x18-byte frame keeps $ra plus the $s0 that holds the 0xFF the two index tests
 * of sub-state 3 compare against.
 * Shape notes from the original bytes: sub-states 2 to 4 reproduce the object
 * row seeding of the exact sibling at 0x801FD400 statement for statement, down
 * to the per-cell reload of the published D_801FEA6C index for the first two
 * row cells and the single scaled offset the remaining stores reuse, while
 * sub-state 3 first block and sub-state 0 take the exact sibling at 0x801FD1A4
 * form of the claimed row held in one pointer. The release tail follows the
 * exact sibling at 0x801FC21C. The original ends sub-states 3 and 5 in one body
 * that psyq gcc merged: both write the sub-state byte from a constant register
 * and then raise the progress byte through one materialised address, so the
 * shared block is reached with that constant in $a0. The measured body keeps
 * that merge, and its extra words come from the frame recorded below.
 * @status partial
 * @match 89.14
 * @residual Saved-register pair exchanged: the measured body keeps the address
 * of the 0x74-byte table base D_80143FC8 in $s0 across the two claim calls of
 * sub-state 3, so its 0x30-byte frame saves $ra, $s0 and $s1 and the 0xFF the
 * two index tests compare against moves to $s1, while the original materialises
 * that base inline in $v1 at the sub-state 3 store group and keeps 0xFF in $s0
 * inside a 0x18-byte frame. The same live base also turns the first row store of
 * the second sub-state 3 seed into a register-based `sb $v1,0($v0)` where the
 * original keeps `lui $at,%hi` / `addu $at,$at,$v0` / `sb %lo($at)`. Logic,
 * sub-state dispatch, statement order, literals and bindings all match; the
 * other differing words are the four prologue/epilogue slots of that frame and
 * the intra-function branch targets shifted by the instructions it adds. The
 * switch-table words also stay different until the reviewed .rodata placement
 * for 0x801F6CBC is restored, which the comparator only accepts once the body
 * lays out like the original.
 */
void audioFxObjectRows(
    void) {
  switch (D_80146875) {
    case 0: {
      u8 selection;

      selection = D_80145029;
      func_8015D4F8(D_80181EBA[selection * 4], D_80181EBB[selection * 4], 0x64,
                    0x20);
      D_80146876 = 0x20;
      D_80146875 = 1;
      break;
    }

    case 1: {
      u16* count;
      u16 countdown;

      count = &D_80146876;
      countdown = *count;
      *count = countdown - 1;
      if (countdown != 0u) {
        break;
      }

      func_80161CD0(D_80145029, 0x64, 0x20);
      g_ScenarioProgress = 0;
      func_801BE1B0(0);
      D_80146875 = 2;
      break;
    }

    case 2: {
      u8 object_index;
      u32 offset;

      if (g_ScenarioProgress != 1u) {
        break;
      }

      object_index = func_8019601C();
      D_801FEA6C = object_index;
      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)D_801FEA6C * 0x74u] = 0x13u;
        offset = (u32)D_801FEA6C * 0x74u;
        *(s32*)(D_8014402C + offset) = -0x318;
        *(s32*)(D_80144030 + offset) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + offset) = (s32)(s16)D_801492DC;
        D_80143FD1[offset] = 0x1eu;
      }

      D_80146875 = 3;
      break;
    }

    case 3: {
      u8* progress;
      u8 object_index;
      u32 offset;
      s32 shared_half;

      if ((D_80143FC8[(u32)D_801FEA6C * 0x74u] & 1u) != 0u) {
        break;
      }

      object_index = func_8019601C();
      D_1F800000 = object_index;
      if (object_index != 0xffu) {
        u8* row;

        row = D_80143FC8 + (u32)object_index * 0x74u;
        row[0] = 1;
        row[5] = 0x5f;
        row[1] = 0;
      }

      object_index = func_8019601C();
      D_801FEA6C = object_index;
      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)D_801FEA6C * 0x74u] = 0x13u;
        offset = (u32)D_801FEA6C * 0x74u;
        *(s32*)(D_8014402C + offset) = (s32)(s16)D_801492D8;
        shared_half = D_801492DA;
        *(s32*)(D_80144034 + offset) = 0x1c;
        D_80143FD1[offset] = 0xb4u;
        *(s32*)(D_80144030 + offset) = shared_half;
      }

      progress = &g_ScenarioProgress;
      D_80146875 = 4;
      *progress = *progress + 1;
      break;
    }

    case 4: {
      u8* progress;
      u8 object_index;
      u32 offset;

      if ((D_80143FC8[(u32)D_801FEA6C * 0x74u] & 1u) != 0u) {
        break;
      }

      object_index = func_8019601C();
      D_801FEA6C = object_index;
      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)D_801FEA6C * 0x74u] = 0x13u;
        offset = (u32)D_801FEA6C * 0x74u;
        *(s32*)(D_8014402C + offset) = -0x2aa;
        *(s32*)(D_80144034 + offset) = 0x200;
        *(s32*)(D_80144030 + offset) = 0;
        D_80143FD1[offset] = 0x5au;
      }

      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      func_801BFE34(0x430000, 0x380000, 0x21);
      func_801BE1B0(1);
      D_80146875 = 5;
      break;
    }

    case 5: {
      u8* progress;

      if ((D_80143FC8[(u32)D_801FEA6C * 0x74u] & 1u) != 0u) {
        break;
      }

      progress = &g_ScenarioProgress;
      D_80146875 = 6;
      *progress = *progress + 1;
      break;
    }

    case 6:
      if (g_ScenarioProgress != 5u) {
        break;
      }

      func_801C7A54(0x21);
      D_80146875 = 7;
      break;

    case 7:
      break;

    case 8:
      g_ScenarioProgress = 0;
      func_8015C058();
      func_8015B580(D_8014686C, 8);
      D_80146874 = 0;
      D_80146875 = 0;
      D_80146258 &= 0xfff7u;
      break;
  }
}
