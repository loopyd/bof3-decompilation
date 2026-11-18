#include "bof3/bof3.h"

extern u8 D_1F800000;
extern u8 D_80143F1F;
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern u16 D_80143C40;
extern u16 D_80146258;
extern s16 D_801492DA;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;
extern u32 D_8014686C;
extern u8 D_8014832E;
extern u8 g_ScenarioProgress;

void func_8014ECAC(u16 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015D404(u32 arg0, s32 arg1);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
s32 func_8019601C(void);
void func_8019FA28(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
void func_801BE1B0(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C601C(u32 arg0);
void func_801C7A54(s32 arg0);

/* @source 0x801FAC54
 * @behavior Slot 22 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2E0) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot. It walks the shared sub-state byte D_80146875
 *   (0x80146875) through the eleven-slot absolute jump table the payload keeps
 *   at 0x801F6FA4: slots 0x0A through 0x11 hold bodies, the table words of
 *   sub-states 0x12, 0x13 and 0x14 are the ones that point at the body exit, and
 *   a sub-state the unsigned bound of 11 rejects returns without touching
 *   shared state. Sub-state 0x0A advances to 0x0B, sets scenario flag bit 0x1D
 *   of the shared flag word D_8014686C (0x8014686C) through func_8015B580 and
 *   seeds the front selector context func_8019FA28 with 0x0B, 0x200000,
 *   0x110000 and kind 0x80. Sub-state 0x0B sets front-end mode 5 through
 *   func_8014ECAC, stores 0x1F in the shared effect-bank byte D_8014832E
 *   (0x8014832E) and advances to 0x0C.
 *   Sub-states 0x0C and 0x0D claim a front-end record through the shared
 *   allocator func_8019601C, publish that index in the scratchpad byte
 *   0x1F800000 and, unless it reads 0xFF, seed the 0x74-byte row of D_80143FC8
 *   (0x80143FC8) the index selects: +0 reads 1, +5 reads 0x13, the word at
 *   +0x64 reads -0x4D0 (-0x2AA on 0x0D), the signed halfword D_801492DA
 *   (0x801492DA) lands at +0x68, the word at +0x6C reads 0x2B0 (0x200 on 0x0D)
 *   and the byte at +9 reads 0xB4 (0x40 on 0x0D); the sub-state byte then
 *   advances to the next slot. 0x0C enters only once the shared gate halfword
 *   D_80143C40 (0x80143C40) reads zero and first stores 1 in the progress byte
 *   g_ScenarioProgress (0x80146864) and runs func_801C601C with 0, while 0x0D
 *   waits for that progress byte to read 2 and claims directly.
 *   Sub-state 0x0E, once the progress byte reads 3, raises mode bit 8 of the
 *   shared mode word D_80146258 (0x80146258) and runs func_801BE1B0 with 1,
 *   then advances to 0x0F. Sub-state 0x0F, once the progress byte reads 1
 *   through an address-taken pointer, calls func_8015D404 with two zero
 *   arguments, queues the front-end cue func_80161C20(0x0D, 0x64, 8), advances
 *   to 0x10, stores 0x1E in the progress byte, seeds func_8019FA28 with 0x0B,
 *   0x220000, 0x110000 and kind 0x82, and writes 0x0D to the shared prompt byte
 *   D_80143F1F (0x80143F1F).
 *   Sub-state 0x10, once the progress byte reads 0x27, takes the front-end word
 *   pair 0x1E0000 and 0x110000 with 0x0D to func_801BFE34, runs func_801C7A54
 *   with 0x0D and advances to 0x11. Sub-state 0x11, once the progress byte reads
 *   0x3C, clears the sub-state byte, D_80146874 (0x80146874) and D_80146866
 *   (0x80146866), sets scenario flag bit 0x1E through func_8015B580, seeds
 *   func_8019FA28 with 3, 0x4F0000, 0x1A0000 and kind 0x80, and writes 0xFF to
 *   D_80143F1F, which ends the chain.
 *   It takes no arguments and returns nothing; its 0x18-byte frame keeps $ra
 *   and $s0, the sub-state byte is read once before the frame is grown, each
 *   claim block holds the scratchpad base in $a0 and rebuilds every row offset
 *   after the first from a fresh read of that byte, and the 1 stored in the
 *   progress byte stays in $s0 for the first row cell.
 *
 * Name evidence: no in-payload `jal` targets 0x801FAC54 and the only payload
 * word equal to that address is the chain-table pointer at 0x801FE2E0, so the
 * span is a progress-chain step handler and takes a target-qualified step name.
 * The ladder over the shared sub-state byte, the dispatch prologue and the
 * tail merge of the two prompt-byte stores are the shape of the exact sibling
 * runSubstateLadderSeedingTwoFrontEndRecordRowsThenReleaseScenarioScena0200_801F9800
 * (0x801F9800, slot 16 of the same run). The scratchpad publication, the
 * 0x80143FC8 row cells and the signed halfword store are that same sibling's
 * row seeding block, and the helper idiom of the arms is the shape of the exact
 * sibling runPrimaryState8SubstateSequenceScenarioScena1100_801F9184
 * (0x801F9184). The sub-state literals, the progress gates, the helper
 * arguments, the row literals and the flag bits are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801FAC54-0x801FB0E7 is one
 * complete function of 0x494 bytes with one prologue at 0x801FAC5C and a single
 * `jr $ra` epilogue at 0x801FB0E0, so neither the data nor the carve lever
 * applies. Clone is declined on payload evidence: a scan of every payload under
 * out/binaries for these 1172 bytes matched this span alone, and a second scan
 * that masks the `lui`, `j` and `jal` immediates matched this span alone as
 * well, so no byte-identical or relocation-different body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSubstateLadderClaimFrontEndRecordsSeedSelectorContextsThenResetScenarioScena0200_801FAC54(void) {
  u8* scratch;
  u8* progress;
  u8 object_index;

  switch (D_80146875) {
  case 0xA:
    D_80146875 = 0xB;
    func_8015B580(D_8014686C, 0x1D);
    func_8019FA28(0xB, 0x200000, 0x110000, 0x80);
    break;
  case 0xB:
    func_8014ECAC(5);
    D_8014832E = 0x1F;
    D_80146875 = 0xC;
    break;
  case 0xC:
    if (D_80143C40 != 0) {
      break;
    }
    g_ScenarioProgress = 1;
    func_801C601C(0);
    object_index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = object_index;
    if (object_index == 0xFF) {
      break;
    }
    D_80143FC8[(u32)object_index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
    *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x4D0;
    *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
    *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x2B0;
    D_80143FD1[(u32)*scratch * 0x74u] = 0xB4u;
    D_80146875 = 0xD;
    break;
  case 0xD:
    if (g_ScenarioProgress != 2) {
      break;
    }
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
    D_80143FD1[(u32)*scratch * 0x74u] = 0x40u;
    D_80146875 = 0xE;
    break;
  case 0xE:
    if (g_ScenarioProgress != 3) {
      break;
    }
    D_80146258 = (u16)(D_80146258 | 8u);
    func_801BE1B0(1);
    D_80146875 = 0xF;
    break;
  case 0xF:
    progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    func_8015D404(0, 0);
    func_80161C20(0x0D, 0x64, 8);
    D_80146875 = 0x10;
    *progress = 0x1E;
    func_8019FA28(0xB, 0x220000, 0x110000, 0x82);
    D_80143F1F = 0x0D;
    break;
  case 0x10:
    if (g_ScenarioProgress != 0x27) {
      break;
    }
    func_801BFE34(0x1E0000, 0x110000, 0x0D);
    func_801C7A54(0x0D);
    D_80146875 = 0x11;
    break;
  case 0x11:
    if (g_ScenarioProgress != 0x3C) {
      break;
    }
    D_80146875 = 0;
    D_80146874 = 0;
    D_80146866 = 0;
    func_8015B580(D_8014686C, 0x1E);
    func_8019FA28(3, 0x4F0000, 0x1A0000, 0x80);
    D_80143F1F = 0xFF;
    break;
  case 0x12:
  case 0x13:
  case 0x14:
    break;
  }
}
