#include "bof3/bof3.h"

/* Shared gate halfword of the main executable: sub-states 2, 4 and 7 wait for
 * it to read zero before acting. */
extern u16 D_80143C40;
/* Shared front-end mode byte of the main executable: mode 2 is busy, so
 * sub-state 3 stops until the byte leaves 2, sub-state 5 waits for it to reach
 * 2 and the closing arm returns to sub-state 5 only while it reads anything
 * else. */
extern u8 D_80143BB0;
/* Shared scenario flag word of the main executable, passed whole to the shared
 * flag-bit setter. */
extern u32 D_8014686C;
/* Shared mode halfword of the main executable: the closing arm clears its 0x100
 * bit. */
extern u16 D_80146258;
extern u8 D_80146874;
extern u8 D_80146875;
/* Shared halfword countdown sub-state 1 arms with 0x3C and sub-state 6 ticks. */
extern u16 D_80146876;
/* Shared effect bank byte of the main executable, cleared by sub-state 2. */
extern u8 D_8014832E;
/* Shared tint channel byte sub-state 3 saturates. */
extern u8 D_80143F1E;
/* Shared scenario progress byte of the main executable: the waiting arms
 * compare it against a constant and the advancing arms add one to it. */
extern u8 g_ScenarioProgress;
/* Shared front-end selector halfword the closing arm watches for 0x33. */
extern u16 D_801490A8;

void func_8014ECAC(u16 local_mode);
void func_80150224(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015C088(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b, u32 kind);
void func_801BE1B0(u32 arg0);

/* @source 0x801FA0A4
 * @behavior Word 4 of the nine-word primary state handler table at 0x801FCFE0
 * that dispatchPrimaryStateHandlerScenarioScena0300_801F92F4 (0x801F92F4)
 * indexes with the signed shared primary state byte D_80146874 and invokes with
 * no arguments, so no in-image jal targets this address and that table word
 * 0x801FCFB0 is its only reference in this payload. It switches on the shared
 * sub-state byte D_80146875 through the eleven-entry absolute jump table
 * jtbl_801F6CF4_main at 0x801F6CF4 in this payload leading table data, bounded
 * by sltiu against 0xB, and any other value returns without a store. Every
 * shared cell is written through its own canonical label, so the original
 * re-forms the shared base for each store; the two arms that read and rewrite
 * the progress byte materialise that base inside their own block, while the arm
 * that holds it across a call keeps one base for the whole block.
 * Sub-state 0 runs the main-exe helper func_801BE1B0(0) and advances to 1.
 * Sub-state 1 waits until the progress byte reads 4, then selects local mode 0
 * through func_8014ECAC, arms the countdown D_80146876 with 0x3C and advances
 * to 2. Sub-state 2 waits for the gate halfword to clear, requests action 0x20
 * through func_80150224, parks the front-end mode byte at 2, clears the effect
 * bank byte and advances to 3. Sub-state 3 refuses to act while the mode byte
 * still reads 2; otherwise it requests front selector context seed 0x25 with
 * the context words 0x2D0000 and 0x280000 and kind 0x81 through func_8019FA28,
 * saturates the tint channel byte D_80143F1E and advances to 4. Sub-state 4,
 * once the gate halfword reads zero, adds one to the progress byte and releases
 * through func_8015C058 before advancing to 5. Sub-state 5 waits for mode 2 and
 * for the selector halfword D_801490A8 to read 0x33, then jumps the sub-state
 * to 10. Sub-state 6 ticks the countdown while it is non-zero; once it reads
 * zero it runs func_8015C088, sets scenario flag bit 0x15 of the shared flag
 * word D_8014686C through func_8015B580, stores 1 in the progress byte,
 * requests selector context seed 0x25 with the context words 0x2D0000 and
 * 0x280000 and kind 0x82 through func_8019FA28 and advances to 7. Sub-state 7,
 * once the gate halfword reads zero, advances to 8 and adds one to the progress
 * byte. Sub-state 8 waits for progress 0x0F and then clears the progress byte,
 * releases through func_8015C058, and resets the primary state byte and the
 * sub-state byte to 0. Sub-state 9 does nothing. Sub-state 10, while the mode
 * byte reads anything but 2, returns to sub-state 5 and clears bit 0x100 of the
 * shared mode halfword D_80146258.
 * It takes no arguments and returns nothing; its 0x18-byte frame keeps $ra
 * across the calls and no other register. Its 0x2A8 bytes have no second copy in
 * the tree: a span search over every payload under out/binaries finds this its
 * only occurrence, and the body also embeds this payload's own jump-table base
 * 0x801F6CF4, so the clone lever is declined and every literal here was written
 * from this span's own bytes. The nearest shape donor is the same target's word
 * 7 handler runPrimaryState7SubstateSequenceScenarioScena0300_801FB268
 * (0x801FB268): its jump-table switch, gate-halfword waits, countdown tick,
 * mode-byte guards and closing flag clear supplied the statement order and
 * operand spelling of the arms here, and its per-arm base materialisation
 * supplied the block-scoped progress bases.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState4SubstateSequenceScenarioScena0300_801FA0A4(void) {
  u8 *progress;
  u16 *countdown;
  u8 value;

  switch (D_80146875) {
  case 0:
    func_801BE1B0(0);
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress != 4u) {
      break;
    }
    func_8014ECAC(0);
    D_80146876 = 0x3C;
    D_80146875 = 2;
    break;
  case 2:
    if (D_80143C40 != 0u) {
      break;
    }
    func_80150224(0x20);
    D_80143BB0 = 2;
    D_8014832E = 0;
    D_80146875 = 3;
    break;
  case 3:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8019FA28(0x25, 0x2D0000, 0x280000, 0x81);
    D_80143F1E = 0xFF;
    D_80146875 = 4;
    break;
  case 4:
    if (D_80143C40 != 0u) {
      break;
    }
    {
      u8 *progress = &g_ScenarioProgress;

      value = *progress + 1;
      *progress = value;
    }
    func_8015C058();
    D_80146875 = 5;
    break;
  case 5:
    if (D_80143BB0 != 2u) {
      break;
    }
    if (D_801490A8 != 0x33u) {
      break;
    }
    D_80146875 = 0xA;
    break;
  case 6:
    countdown = &D_80146876;
    if (*countdown != 0u) {
      *countdown = *countdown - 1;
      break;
    }
    func_8015C088();
    func_8015B580(D_8014686C, 0x15);
    g_ScenarioProgress = 1;
    func_8019FA28(0x25, 0x2D0000, 0x280000, 0x82);
    D_80146875 = 7;
    break;
  case 7:
    if (D_80143C40 != 0u) {
      break;
    }
    {
      u8 *progress = &g_ScenarioProgress;
      u8 next;

      next = *progress + 1;
      D_80146875 = 8;
      *progress = next;
    }
    break;
  case 8:
    progress = &g_ScenarioProgress;
    if (*progress != 0x0Fu) {
      break;
    }
    *progress = 0;
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 9:
    break;
  case 10:
    if (D_80143BB0 != 2u) {
      D_80146875 = 5;
      D_80146258 = D_80146258 & 0xFEFFu;
    }
    break;
  }
}
