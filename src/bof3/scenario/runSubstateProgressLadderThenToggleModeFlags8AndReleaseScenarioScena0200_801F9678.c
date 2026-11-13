#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);

/* @source 0x801F9678
 * @behavior Slot 15 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2C4) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot. It walks the shared sub-state byte D_80146875
 *   (0x80146875) through the five-slot absolute jump table the payload keeps at
 *   0x801F6E0C: slots 0 through 4 are live and a sub-state of 5 or more, which
 *   the unsigned bound rejects, returns without touching the shared state. Slot
 *   0 raises bit 0x8 of the shared halfword mode word D_80146258 (0x80146258)
 *   with lhu/ori/sh, runs the shared GAME.EMI helper func_801BE1B0 with 0x1A
 *   and func_801C601C with 1, then advances the sub-state byte to 1. Slot 1,
 *   once the shared progress byte g_ScenarioProgress (0x80146864) reads 0x15,
 *   runs func_801BE1B0 with 5 and advances to 2. Slot 2, once that byte reads
 *   0x1F, takes the front-end word pair 0x50000 and 0x370000 with 9 to
 *   func_801BFE34, runs func_801C7A54 with 9 and advances to 3. Slot 3, once
 *   the progress byte reads 0x20, runs func_801BE1B0 with 0x15 and advances to
 *   4. Slot 4, once the progress byte reads 0x25, sets flag bit 0x11 of the
 *   shared scenario flag word D_8014686C (0x8014686C) through func_8015B580,
 *   flips the same mode bit 0x8 with lhu/xori/sh, runs the shared overlay
 *   release helper func_8015C058, clears the shared bytes 0x80146865, 0x80146866
 *   and 0x80146867, and returns the overlay to sub-state 0 and primary state 0
 *   by clearing D_80146875 and D_80146874 (0x80146874), in that order. It takes
 *   no arguments and returns nothing; its 0x18-byte frame keeps only $ra, the
 *   sub-state byte is read once before the frame is grown, the two waits on the
 *   progress byte re-read it through the address each comparison took, and the
 *   release helper's delay slot stays a nop. The role of the toggled mode bit
 *   and of the cleared bytes remains unknown.
 *
 * Name evidence: no in-payload `jal` targets 0x801F9678, and the only payload
 * word equal to that address is the chain-table pointer at 0x801FE2C4, so the
 * span is a progress-chain step handler and takes a target-qualified step name.
 * The walk is the shape of the exact sibling
 * runSubstateProgressFlagAndFrontEndRecordStepsThenReleaseScenarioScena0200_801FB6C4
 * (0x801FB6C4, slot 28 of the same run), which also gates sub-states on the
 * progress byte, sets a flag bit of D_8014686C through func_8015B580, releases
 * the overlay through func_8015C058 and clears the shared bytes 0x80146865
 * through 0x80146867 before returning to sub-state 0 and primary state 0, and
 * of the exact sibling
 * waitFlag5ThenToggleModeFlags36AndRequestPrimaryState4Substate7ScenarioScena0200_801FD1DC
 * (0x801FD1DC), which flips bits of the same mode word. The mode-bit constant,
 * the progress gates, the front-end pair and the cleared bytes are this span's
 * own.
 *
 * Boundary and lever evidence: the payload span 0x801F9678-0x801F97FF is one
 * complete function of 0x188 bytes with a single `jr $ra` epilogue, so it is
 * neither data nor a fragment and no data or carve lever applies. The dispatch
 * table is 5 words, 0x801F6E0C through 0x801F6E1F, and word 5 would sit at
 * 0x801F6E20, which is not a case address. Clone is declined on payload
 * evidence: a scan of every payload under out/binaries for these 392 bytes
 * matched this span alone, and a second scan that masks only the address
 * immediates matched this span alone as well, so no byte-identical or
 * relocation-different sibling body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSubstateProgressLadderThenToggleModeFlags8AndReleaseScenarioScena0200_801F9678(void) {
  switch (D_80146875) {
  case 0:
    D_80146258 |= 8;
    func_801BE1B0(0x1A);
    func_801C601C(1);
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress == 0x15) {
      func_801BE1B0(5);
      D_80146875 = 2;
    }
    break;
  case 2:
    if (g_ScenarioProgress == 0x1F) {
      func_801BFE34(0x50000, 0x370000, 9);
      func_801C7A54(9);
      D_80146875 = 3;
    }
    break;
  case 3:
    if (g_ScenarioProgress == 0x20) {
      func_801BE1B0(0x15);
      D_80146875 = 4;
    }
    break;
  case 4:
    if (g_ScenarioProgress == 0x25) {
      func_8015B580(D_8014686C, 0x11);
      D_80146258 ^= 8;
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
