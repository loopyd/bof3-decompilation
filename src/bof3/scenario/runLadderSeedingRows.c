#include "bof3/bof3.h"

extern u8 D_1F800000;
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern s16 D_801492DA;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;
extern u32 D_8014686C;
extern u8 g_ScenarioProgress;

s32 func_8019601C(void);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);

/* @source 0x801F9800
 * @behavior Slot 16 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2C8) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot. It walks the shared sub-state byte D_80146875
 *   (0x80146875) through the seven-slot absolute jump table the payload keeps at
 *   0x801F6E24: slots 0 through 6 are live and a sub-state of 7 or more, which
 *   the unsigned bound rejects, returns without touching the shared state. Slot
 *   0 runs the shared GAME.EMI helper func_801BE1B0 with 6 and func_801C601C
 *   with 2, then advances the sub-state byte to 1. Slot 1, once the shared
 *   progress byte g_ScenarioProgress (0x80146864) reads 0x2B, takes the
 *   front-end word pair 0x100000 and 0x70000 with 0xA to func_801BFE34 and
 *   advances to 2. Slot 2 asks func_8019601C for an object index, publishes that
 *   byte in scratchpad 0x1F800000 and, unless it reads 0xFF, seeds the objects
 *   0x74-byte row at 0x80143FC8: +0 reads 1, +5 reads 0x13, +9 reads 0x14, the
 *   word at +0x64 reads -0x2BA, the signed halfword D_801492DA (0x801492DA)
 *   lands at +0x68 and the word at +0x6C reads 0x260; the sub-state byte then
 *   advances to 3. Slot 3, once the progress byte reads 0x2F, runs
 *   func_801C7A54 with 0xA and advances to 4. Slot 4, once the progress byte
 *   reads 0x30, runs func_801BE1B0 with 0x16 and advances to 5. Slot 5 publishes
 *   another object index the same way and seeds that objects row with the same
 *   cells except the word -0x2AA at +0x64 and the word 0x200 at +0x6C, then
 *   advances to 6. Slot 6, once the progress byte reads 0x31, sets scenario flag
 *   bits 0x12 and 0x13 of the shared flag word D_8014686C (0x8014686C) through
 *   func_8015B580, releases the overlay through the shared release helper
 *   func_8015C058 and clears the shared bytes 0x80146865, 0x80146866 and
 *   0x80146867, the sub-state byte and D_80146874 (0x80146874), in that order.
 *   It takes no arguments and returns nothing; its 0x18-byte frame keeps only
 *   $ra, the sub-state byte is read once before the frame is grown, and every
 *   wait on the progress byte re-reads it through the address its comparison
 *   took. The scratchpad base stays in $a0 across both seeding blocks and each
 *   row cell after the first rebuilds its 0x74-scaled offset from a fresh read
 *   of that byte, while the first cell reuses the masked index from the call.
 *   The role of the published object index and of the row cells remains unknown.
 *
 * Name evidence: no in-payload `jal` targets 0x801F9800, and the only payload
 * word equal to that address is the chain-table pointer at 0x801FE2C8, so the
 * span is a progress-chain step handler and takes a target-qualified step name.
 * The ladder over the shared sub-state byte and the dispatch prologue are the
 * shape of the exact sibling
 * runLadderToggleRelease
 * (0x801F9678, slot 15 of the same run). The scratchpad publication, the
 * 0x80143FC8 row cells and the signed halfword store are the shape of the exact
 * sibling
 * seedTwoObjectRowsAtProgress6ThenSetFlagBit14AndResetHandlerScenarioScena0800_801FD400
 * (0x801FD400), whose 0x74-byte row seeding block repeats these cells in this
 * order. The progress gates, the helper arguments, the row literals and the
 * flag bits are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801F9800-0x801F9BCF is one
 * complete function of 0x3D0 bytes with one prologue and a single `jr $ra`
 * epilogue, so neither the data nor the carve lever applies. Clone is declined
 * on payload evidence: a scan of every payload under out/binaries for these 976
 * bytes matched this span alone, and a second scan that masks the `lui` and
 * `jal` immediates matched this span alone as well, so no byte-identical or
 * relocation-different body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runLadderSeedingRows(void) {
  u8* scratch;
  u8 object_index;

  switch (D_80146875) {
  case 0:
    func_801BE1B0(6);
    func_801C601C(2);
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress == 0x2B) {
      func_801BFE34(0x100000, 0x70000, 0xA);
      D_80146875 = 2;
    }
    break;
  case 2:
    object_index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = object_index;
    if (object_index == 0xFF) {
      break;
    }
    D_80143FC8[(u32)object_index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
    *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2BA;
    *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
    *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x260;
    D_80143FD1[(u32)*scratch * 0x74u] = 0x14u;
    D_80146875 = 3;
    break;
  case 3:
    if (g_ScenarioProgress == 0x2F) {
      func_801C7A54(0xA);
      D_80146875 = 4;
    }
    break;
  case 4:
    if (g_ScenarioProgress == 0x30) {
      func_801BE1B0(0x16);
      D_80146875 = 5;
    }
    break;
  case 5:
    object_index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = object_index;
    if (object_index == 0xFF) {
      break;
    }
    D_80143FC8[(u32)object_index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
    *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
    *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
    *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
    D_80143FD1[(u32)*scratch * 0x74u] = 0x14u;
    D_80146875 = 6;
    break;
  case 6:
    if (g_ScenarioProgress == 0x31) {
      func_8015B580(D_8014686C, 0x12);
      func_8015B580(D_8014686C, 0x13);
      func_8015C058();
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  }
}
