#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80146258;
extern u16 D_80143C40;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;
extern u8 D_80149332;

/* Shared front-end mode setter of the main executable. */
void func_8014ECAC(u16 arg0);
/* Shared scenario flag bit-set helper of the main executable: it takes the
 * active flag word and the bit id. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper of the main executable. */
void func_8015C058(void);
/* Shared front-end cue dispatcher of the main executable. */
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
/* Shared main-executable helper at 0x801A78F8; its role is not established. */
void func_801A78F8(void);
/* Shared main-executable helper at 0x801C16B8; its role is not established. */
void func_801C16B8(void);
/* Shared main-executable helper at 0x801C601C; its role is not established. */
void func_801C601C(u32 arg0);

/* @source 0x801FB0F0
 * @behavior Slot 24 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2E8) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot. It walks the shared sub-state byte D_80146875
 *   (0x80146875) through the eight-slot absolute jump table the payload keeps at
 *   0x801F6FD4: slots 0, 1, 5, 6 and 7 are live, slots 2, 3 and 4 point at the
 *   epilogue, and a sub-state of 8 or more that the unsigned bound rejects
 *   returns without touching the shared state. Slot 0 arms the effect bank
 *   D_8014832E (0x8014832E) with 0x1F, queues the front-end cue
 *   func_80161C20(0xF, 0x64, 8), runs func_801C601C with 0 and advances the
 *   sub-state byte to 1. Slot 1, once the shared byte D_80149332 (0x80149332)
 *   reads zero and the progress byte g_ScenarioProgress (0x80146864) reads 3,
 *   runs func_801C16B8, flips mode bit 0x8 of the shared halfword mode word
 *   D_80146258 (0x80146258) with lhu/xori/sh, sets flag bit 0x1F of the shared
 *   scenario flag word D_8014686C (0x8014686C) through func_8015B580, clears
 *   the progress byte and the shared bytes 0x80146865 and 0x80146867, raises
 *   mode bit 0x80, releases the overlay through func_8015C058 and returns to
 *   sub-state 0 and primary state 0 by clearing D_80146875 and D_80146874
 *   (0x80146874), in that order. Slot 5, once the progress byte reads 1, sets
 *   the front-end mode 5 through func_8014ECAC, arms the effect bank with 0x1F
 *   and advances the sub-state byte to 6. Slot 6, while the gate word
 *   D_80143C40 (0x80143C40) is clear, stores 2 in the progress byte and
 *   advances the sub-state byte to 7. Slot 7, once the progress byte reads
 *   0xF, sets flag bit 0x20 of the flag word through func_8015B580, clears the
 *   progress byte and the shared bytes 0x80146865, 0x80146866 and 0x80146867,
 *   flips mode bit 0x80, releases the overlay through func_8015C058, returns
 *   to sub-state 0 and primary state 0 and finally runs the shared helper
 *   func_801A78F8. It takes no arguments and returns nothing; its 0x18-byte
 *   frame keeps $ra and $s0, the sub-state byte is read once before the frame
 *   is grown, the $s0 held address of the progress byte serves both read sites
 *   and survives the flag helper call in slots 1 and 7 while the single-read
 *   progress tests in slots 5 and 6 materialise the 0x8014 page in a temporary
 *   register, the release helper's delay slot stays a nop and the tail
 *   `jr $ra` keeps its nop delay slot.
 *
 * Name evidence: no in-payload `jal` targets 0x801FB0F0, and the only payload
 *   word equal to that address is the chain-table pointer at 0x801FE2E8, word
 *   24 of the run dispatchProgressHandlerScenarioScena0200_801F719C indexes, so
 *   the span is a progress-chain step handler and takes a target-qualified step
 *   name. The two-step queue-and-release walk, the effect-bank arm with 0x1F,
 *   the flag-word bit set through func_8015B580 and the return to sub-state 0
 *   and primary state 0 are the shape of the exact siblings
 *   runSubstateProgressLadderThenToggleModeFlags8AndReleaseScenarioScena0200_801F9678
 *   (0x801F9678, slot 15 of the same run) and
 *   runSubstateProgressFlagAndFrontEndRecordStepsThenReleaseScenarioScena0200_801FB6C4
 *   (0x801FB6C4, slot 28 of the same run); the slot layout, the cue constants,
 *   the progress gates 1, 3 and 0xF, the flag bits 0x1F and 0x20 and the mode
 *   bits 0x8 and 0x80 are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801FB0F0-0x801FB2F7 is one
 *   complete function of 0x208 bytes with a single `jr $ra` epilogue, so it is
 *   neither data nor a fragment and no data or carve lever applies. The
 *   dispatch table is 8 words, 0x801F6FD4 through 0x801F6FF3, and word 8 would
 *   sit at 0x801F6FF4, which is not a case address. Clone is declined on
 *   payload evidence: a scan of every payload under out/binaries for these 520
 *   bytes matched this span alone, and a second scan that masks the address,
 *   immediate and branch fields matched this span alone as well, so no
 *   byte-identical or relocation-different sibling body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSubstateSequenceQueueCueArmEffectBankSeedProgressAndReleaseScenarioScena0200_801FB0F0(void) {
  switch (D_80146875) {
  case 0:
    D_8014832E = 0x1F;
    func_80161C20(0xF, 0x64, 8);
    func_801C601C(0);
    D_80146875 = 1;
    break;
  case 1:
    if (D_80149332 == 0) {
      u8* progress = &g_ScenarioProgress;

      if (*progress == 3) {
        func_801C16B8();
        D_80146258 ^= 8;
        func_8015B580(D_8014686C, 0x1F);
        *progress = 0;
        D_80146865 = 0;
        D_80146867 = 0;
        D_80146258 |= 0x80;
        func_8015C058();
        D_80146875 = 0;
        D_80146874 = 0;
      }
    }
    break;
  case 5:
    if (g_ScenarioProgress == 1) {
      func_8014ECAC(5);
      D_8014832E = 0x1F;
      D_80146875 = 6;
    }
    break;
  case 6:
    if (D_80143C40 == 0) {
      g_ScenarioProgress = 2;
      D_80146875 = 7;
    }
    break;
  case 7: {
    u8* progress = &g_ScenarioProgress;

    if (*progress == 0xF) {
      func_8015B580(D_8014686C, 0x20);
      *progress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146258 ^= 0x80;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
      func_801A78F8();
    }
    break;
  }
  }
}
