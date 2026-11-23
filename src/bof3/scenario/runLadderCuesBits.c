#include "bof3/bof3.h"

extern u8 D_80143F1F;
extern u8 D_80144F5A;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;
extern u32 D_8014686C;
extern u8 g_ScenarioProgress;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015DF18(u32 arg0);
void func_8019FA28(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
void func_801BE1B0(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C601C(u32 arg0);
void func_801C7A54(s32 arg0);

/* @source 0x801F9BD0
 * @behavior Slot 17 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2CC) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot. It walks the shared sub-state byte D_80146875
 *   (0x80146875) through the eleven-slot absolute jump table the payload keeps
 *   at 0x801F6E44: slots 0 through 10 are declared, so a sub-state of 11 or
 *   more, which the unsigned bound rejects, and the empty slot 9 both return
 *   without touching shared state. Slot 0 arms the shared GAME.EMI helper func_801BE1B0
 *   with 9 when the shared party byte D_80144F5A (0x80144F5A) is non-zero and
 *   with 8 otherwise, then advances the sub-state to 1. Slot 1, once the shared
 *   progress byte g_ScenarioProgress (0x80146864) reads 0x3E, runs
 *   func_801C601C with 3 and advances to 2. Slot 2, once that byte reads 0x41,
 *   queues the front-end cue 0x204 through func_8015DF18 and advances to 3.
 *   Slot 3, once that byte reads 0x42, sets scenario flag bit 0x17 of the
 *   shared flag word D_8014686C (0x8014686C) through func_8015B580, takes the
 *   front-end word pair 0x3B0000 and 0x488000 with 0xB to func_801BFE34 and
 *   advances to 4. Slot 4, once that byte reads 0x43, runs func_801C7A54 with
 *   0xB and advances to 5. Slot 5, once that byte reads 0x44, arms
 *   func_801BE1B0 with 0x18 when D_80144F5A is non-zero and with 0x17
 *   otherwise, then advances to 6. Slot 6, once that byte reads 0x46, writes 1
 *   to the shared byte D_80146866 (0x80146866), advances to 7, seeds the front
 *   selector context func_8019FA28 with 0x1B, 0x3B0000 and 0x1A0000 and cue
 *   0x8B when D_80144F5A is non-zero and cue 0x8A otherwise, then writes 0x22
 *   to D_80143F1F (0x80143F1F). Slot 7 runs func_801C601C with 4 and advances
 *   to 8. Slot 8, once that byte reads 0x4B, sets scenario flag bit 0x14
 *   through func_8015B580, writes 1 to D_80146866, advances to 10 and seeds
 *   func_8019FA28 with 0x1B, 0x3C0000, 0x480000 and the same party-dependent
 *   cue pair. Slot 10 releases the overlay through the shared release helper
 *   func_8015C058 and clears the shared bytes 0x80146865 through 0x80146867,
 *   the sub-state byte and D_80146874 (0x80146874), in that order. It takes no
 *   arguments and returns nothing; its 0x18-byte
 *   frame keeps only the return address, the sub-state byte is read once before
 *   the frame is grown, and every wait on the progress byte re-reads it through
 *   the address its comparison took.
 *
 * Name evidence: no payload `jal` reaches 0x801F9BD0 and the only payload word
 * equal to that address is the chain-table pointer at 0x801FE2CC (slot 17), so
 * the span is a progress-chain step handler and takes a target-qualified step
 * name. The ladder over the shared sub-state byte and the release-and-clear
 * tail are the shape of the exact siblings
 * runLadderSeedingRows
 * (0x801F9800, slot 16) and
 * runLadderToggleRelease
 * (0x801F9678, slot 15). The remaining name words follow this overlay own lifts:
 * func_8015DF18 queues a front-end cue, func_8015B580 sets scenario flag bits,
 * func_8019FA28 seeds the front selector context and func_8015C058 releases the
 * overlay.
 *
 * Boundary and lever evidence: the payload span 0x801F9BD0-0x801F9EA3 is one
 * complete function of 0x2D4 bytes with one prologue, one epilogue and a single
 * `jr $ra`, so neither the data nor the carve lever applies. Clone is declined
 * on payload evidence: a scan of the 398 payloads under out/binaries for these
 * 724 bytes matched this span alone, and a second scan that masks the `lui`,
 * `jal` and `j` immediates matched this span alone as well, so no byte
 * identical or relocation-different body exists to clone. The switch table is
 * this span own .rodata at 0x801F6E44, eleven words of 0x2C bytes holding the
 * case entry addresses 0x801F9C04 through 0x801F9E94.
 *
 * Representation note: the first seed wrote each of the four party-dependent
 * sites as `if (D_80144F5A != 0)` with the non-zero literal in the then arm and
 * measured 171/181 instructions with the branch polarity and the two arm stores
 * mirrored at all four sites; the inverted test with the zero-value arm written
 * first is the spelling that matched. The empty slot is 9 and the release block
 * is slot 10, which the table order at 0x801F6E68 and 0x801F6E6C decides.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runLadderCuesBits(void) {
  switch (D_80146875) {
  case 0:
    if (D_80144F5A == 0) {
      func_801BE1B0(8);
    } else {
      func_801BE1B0(9);
    }
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress == 0x3E) {
      func_801C601C(3);
      D_80146875 = 2;
    }
    break;
  case 2:
    if (g_ScenarioProgress == 0x41) {
      func_8015DF18(0x204);
      D_80146875 = 3;
    }
    break;
  case 3:
    if (g_ScenarioProgress == 0x42) {
      func_8015B580(D_8014686C, 0x17);
      func_801BFE34(0x3B0000, 0x488000, 0xB);
      D_80146875 = 4;
    }
    break;
  case 4:
    if (g_ScenarioProgress == 0x43) {
      func_801C7A54(0xB);
      D_80146875 = 5;
    }
    break;
  case 5:
    if (g_ScenarioProgress == 0x44) {
      if (D_80144F5A == 0) {
        func_801BE1B0(0x17);
      } else {
        func_801BE1B0(0x18);
      }
      D_80146875 = 6;
    }
    break;
  case 6:
    if (g_ScenarioProgress == 0x46) {
      D_80146866 = 1;
      D_80146875 = 7;
      if (D_80144F5A == 0) {
        func_8019FA28(0x1B, 0x3B0000, 0x1A0000, 0x8A);
      } else {
        func_8019FA28(0x1B, 0x3B0000, 0x1A0000, 0x8B);
      }
      D_80143F1F = 0x22;
    }
    break;
  case 7:
    func_801C601C(4);
    D_80146875 = 8;
    break;
  case 8:
    if (g_ScenarioProgress == 0x4B) {
      func_8015B580(D_8014686C, 0x14);
      D_80146866 = 1;
      D_80146875 = 0xA;
      if (D_80144F5A == 0) {
        func_8019FA28(0x1B, 0x3C0000, 0x480000, 0x8A);
      } else {
        func_8019FA28(0x1B, 0x3C0000, 0x480000, 0x8B);
      }
    }
    break;
  case 9:
    break;
  case 10:
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  }
}
