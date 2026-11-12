#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80143BB0;
extern u8 D_80146874;
extern u8 D_80146875;

/* Shared scenario flag bit-set helper of the main executable: it takes the
 * active flag word and the bit id. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared front-end mode request of the main executable */
void func_80150224(u32 arg0);
/* Shared overlay release helper of the main executable */
void func_8015C058(void);
/* Shared main-executable slot setter, outside this payload */
extern u8 func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* @source 0x801665A0 */
/* Shared main-executable slot gate, outside this payload */
extern u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3); /* @source 0x801650B4 */

void publishWorkBaseAndSeedFrontEndRecordScenarioScena0200_801FCD14(u8 step);

/* @source 0x801FB6C4
 * @behavior Runs the shared sub-state byte D_80146875 (0x80146875) through the
 *   eighteen-slot absolute jump table the payload keeps at 0x801F704C; slots 0,
 *   1, 10, 15, 16 and 17 are live and every other slot, like a sub-state value
 *   of 18 or more that the unsigned bound rejects, returns without touching the
 *   shared state. Slot 0 seeds the progress byte g_ScenarioProgress (0x80146864)
 *   with 0x15, sets scenario flag bit 0x27 in the flag word D_8014686C
 *   (0x8014686C) through func_8015B580, runs the shared slot setter
 *   func_801665A0 with group 4 and value 2 and advances the sub-state byte to 1.
 *   Slot 1, once the progress byte reads 0x16, finishes that ladder: it clears
 *   the progress byte, the shared bytes 0x80146865, 0x80146866 and 0x80146867,
 *   runs the shared overlay release helper func_8015C058 and returns the overlay
 *   to primary state 0 by clearing the sub-state byte and the primary state byte
 *   D_80146874 (0x80146874), in that order. Slot 10 seeds the progress byte with
 *   0x14, sets flag bit 0x26, runs the shared slot gate func_801650B4 with group
 *   4 and value 2, runs the same release helper, clears the same four shared
 *   bytes and returns to primary state 0 through the same two stores. Slot 15
 *   queues the front-end mode 0x73 through the shared request func_80150224,
 *   arms the shared primary mode byte D_80143BB0 (0x80143BB0) with 2 and
 *   advances the sub-state byte to 0x10. Slot 16, once that mode byte leaves 2,
 *   publishes the front-end record through
 *   publishWorkBaseAndSeedFrontEndRecordScenarioScena0200_801FCD14 with step
 *   0x91, advances the sub-state byte to 0x11 and stores 0x19 in the progress
 *   byte. Slot 17, once the progress byte reads 0x1A, runs the same finish as
 *   slot 1. It takes no arguments and returns nothing; its 0x18-byte frame keeps
 *   only $ra, the sub-state byte is read once before the frame is grown, the two
 *   finish paths share one code block in which each reads the progress byte
 *   through the same address taken above the comparison, and the release
 *   helper's delay slot stays a nop.
 *
 * Name evidence: no in-payload `jal` targets 0x801FB6C4. The only payload word
 * equal to that address is at 0x801FE2F8, word 28 of the 31-word code-pointer
 * run based at 0x801FE288 that dispatchProgressHandlerScenarioScena0200_801F719C
 * indexes with the signed chain byte D_80146872, so the overlay reaches this
 * span as that chain slot. The state walk is the same shape as the exact sibling
 * queueFrontEndMode3EThenWaitModeLeave2ReleaseAndClearStateScenarioScena0200_801F8264
 * (0x801F8264, slot 8 of the same run), which also arms D_80143BB0 with 2 and
 * returns to primary state 0 through D_80146875 and D_80146874, and as the
 * secondary-state controllers of the scena06 and scena16 overlays, so the role
 * word is a sub-state stepper that seeds progress and flag bits, publishes a
 * front-end record and releases the overlay when its progress matches.
 *
 * Boundary and lever evidence: the payload span 0x801FB6C4-0x801FB867 is one
 * complete function of 0x1A4 bytes with a single `jr $ra` epilogue and one
 * shared finish block, so it is neither data nor a fragment and no data or carve
 * lever applies. The dispatch table is 18 words, 0x801F704C through 0x801F7093:
 * word 18 of that run would sit at 0x801F7094, which is the first word of the
 * 31-entry table jtbl_801F7094_main that the next chain slot func_801FB868
 * indexes. Clone is declined on payload evidence: a scan of every payload under
 * out/binaries for these 420 bytes matched this span alone, and a second scan
 * that masks only the absolute immediates matched this span alone as well, so no
 * byte-identical or relocation-different sibling body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSubstateProgressFlagAndFrontEndRecordStepsThenReleaseScenarioScena0200_801FB6C4(void) {
  switch (D_80146875) {
  case 0:
    g_ScenarioProgress = 0x15;
    func_8015B580(D_8014686C, 0x27);
    func_801665A0(4, 2, 1, 0);
    D_80146875 = 1;
    break;
  case 1:
    {
      u8* progress = &g_ScenarioProgress;

      if (*progress == 0x16) {
        *progress = 0;
        D_80146865 = 0;
        D_80146866 = 0;
        D_80146867 = 0;
        func_8015C058();
        D_80146875 = 0;
        D_80146874 = 0;
      }
    }
    break;
  case 10:
    g_ScenarioProgress = 0x14;
    func_8015B580(D_8014686C, 0x26);
    func_801650B4(4, 2, 1, 0);
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 15:
    func_80150224(0x73);
    D_80143BB0 = 2;
    D_80146875 = 0x10;
    break;
  case 16:
    if (D_80143BB0 == 2u) {
      break;
    }
    publishWorkBaseAndSeedFrontEndRecordScenarioScena0200_801FCD14(0x91);
    D_80146875 = 0x11;
    g_ScenarioProgress = 0x19;
    break;
  case 17:
    {
      u8* progress = &g_ScenarioProgress;

      if (*progress == 0x1A) {
        *progress = 0;
        D_80146865 = 0;
        D_80146866 = 0;
        D_80146867 = 0;
        func_8015C058();
        D_80146875 = 0;
        D_80146874 = 0;
      }
    }
    break;
  }
}
