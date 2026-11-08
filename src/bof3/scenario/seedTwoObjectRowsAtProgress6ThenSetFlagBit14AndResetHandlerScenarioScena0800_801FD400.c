#include "bof3/bof3.h"

extern u8 D_1F800000;
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern s16 D_801492DA;
extern u8 D_80146874;
extern u8 D_80146875;
extern u32 D_8014686C;
extern u8 g_ScenarioProgress;
extern u8 D_801FEA6C; /* @source 0x801FEA6C @kind data */

s32 func_8019601C(void);
void func_8015C058(void);
void func_801BE1B0(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FD400
 * @behavior Overlay scenario handler selected by the shared primary state byte
 * D_80146874, driving its own sub-state D_80146875 through the shared
 * sub-state byte. Sub-state 0 runs the shared front-end startup helper
 * func_8015C058 and func_801BE1B0(1) and then returns the overlay to handler 0.
 * Sub-state 0xA asks func_8019601C for an object index, masks it to a byte,
 * publishes it in scratchpad 0x1F800000 and, unless that byte reads 0xFF, seeds
 * the object's 0x74-byte row at 0x80143FC8 (+0 = 1, +5 = 0x13, +9 = 0x20, word
 * -0x2EC at +0x64, the signed halfword D_801492DA at +0x68 and word 0xB6 at
 * +0x6C), then advances to 0xB. Sub-state 0xB waits for the shared progress
 * byte g_ScenarioProgress to read 6 and then claims a second object index,
 * publishes it at 0x801FEA6C and seeds that object's row the same way except
 * for word -0x2AA at +0x64 and word 0x200 at +0x6C, advancing to 0xC. Sub-state
 * 0xC re-reads the published index, and while bit 0 of that row's +0 flag byte
 * is clear it runs func_8015C058, sets scenario flag bit 0x14 through
 * func_8015B580(D_8014686C, 0x14) and returns the overlay to handler 0. Every
 * other sub-state value does nothing. It takes no arguments and returns
 * nothing; the 0x18-byte frame keeps $ra plus $s0, which holds the one
 * materialised D_80146875 byte base that the dispatch load and the three
 * sub-state stores share. The scratchpad pointer stays in $a0 and sub-state 0xA
 * rebuilds its scaled row index from it on every access, while sub-state 0xB
 * re-reads the published 0x801FEA6C byte for each of its first two row cells and
 * then takes that object's scaled offset once, keeping it across the last four
 * row stores; the shared halfword is loaded into its own register ahead of the
 * two stores that follow it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedTwoObjectRowsAtProgress6ThenSetFlagBit14AndResetHandlerScenarioScena0800_801FD400(void) {
  u8* state;
  u8* scratch;
  u8  object_index;
  u32 offset;
  s32 shared_half;

  state = &D_80146875;
  switch (*state) {
    case 0:
      func_8015C058();
      func_801BE1B0(1);
      D_80146874 = 0;
      *state = 0;
      break;

    case 0xa:
      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;
      if (object_index == 0xffu) {
        break;
      }

      D_80143FC8[(u32)object_index * 0x74u] = 1u;
      D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
      *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2ec;
      *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
      *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0xb6;
      D_80143FD1[(u32)*scratch * 0x74u] = 0x20u;
      *state = 0xb;
      break;

    case 0xb:
      if (g_ScenarioProgress != 6u) {
        break;
      }

      object_index = func_8019601C();
      D_801FEA6C = object_index;
      if (object_index == 0xffu) {
        break;
      }

      D_80143FC8[(u32)object_index * 0x74u] = 1u;
      D_80143FCD[(u32)D_801FEA6C * 0x74u] = 0x13u;
      offset = (u32)D_801FEA6C * 0x74u;
      *(s32*)(D_8014402C + offset) = -0x2aa;
      shared_half = D_801492DA;
      *(s32*)(D_80144034 + offset) = 0x200;
      D_80143FD1[offset] = 0x20u;
      *(s32*)(D_80144030 + offset) = shared_half;
      *state = 0xc;
      break;

    case 0xc:
      if ((D_80143FC8[(u32)D_801FEA6C * 0x74u] & 1u) != 0u) {
        break;
      }

      func_8015C058();
      func_8015B580(D_8014686C, 0x14);
      D_80146874 = 0;
      *state = 0;
      break;
  }
}
