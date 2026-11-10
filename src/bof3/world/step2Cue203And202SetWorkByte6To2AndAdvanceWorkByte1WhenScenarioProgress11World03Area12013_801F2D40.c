#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;

/* Scratchpad pointer cell 0x1F800044. The cell sits outside every overlay
 * payload and this target's own symbol map binds it, so the dispatchers at
 * 0x801F2C18 and 0x801F2DE4 read the shared work record the same way. */
extern u8 *D_1F800044;

/* Work-table routine at 0x801F36B0, still unlifted assembly inside this
 * target, so only its prototype is declared. Its head opens a 0x40-byte frame,
 * calls func_801AFE18 and then walks the shared 0x800E4800 work table. This
 * handler ignores whatever it returns. */
void func_801F36B0(void);

/* Same-binary cue dispatcher at 0x8015DF18, which the generated
 * undefined_funcs_auto file of this target binds for comparison. */
extern void func_8015DF18(u32 cue);

/* @source 0x801F2D40
 * @behavior Step 2 of the run this overlay dispatches through the handler
 *           table at 0x801F43C0: the dispatcher at 0x801F2C18 indexes that
 *           table with work-record byte 0x01, and entry 2 of the table holds
 *           this address. The body first runs the target-local work-table
 *           routine func_801F36B0 and ignores its result. It then reads the
 *           shared scenario progress byte g_ScenarioProgress with one
 *           unsigned byte load and compares it against 0x11; only on a match
 *           does it queue cue 0x203 and then cue 0x202 through the
 *           same-binary cue dispatcher func_8015DF18, store 0x02 into byte
 *           0x06 of the scratchpad work record published through pointer cell
 *           0x1F800044, and advance that record's byte 0x01 by one. Byte 0x01
 *           is the byte the table dispatcher indexes, so the increment sends
 *           the next call to entry 3. Each record access loads the pointer
 *           cell D_1F800044 with its own `lui` and offset load, because the
 *           store through the loaded pointer may overwrite the cell. Takes no
 *           arguments, returns nothing, and its 0x18-byte frame holds only the
 *           saved return address at 0x10. The roles of the progress value, of
 *           the two cues and of record byte 0x06 stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void step2Cue203And202SetWorkByte6To2AndAdvanceWorkByte1WhenScenarioProgress11World03Area12013_801F2D40(
    void) {
  func_801F36B0();
  if (g_ScenarioProgress == 0x11) {
    func_8015DF18(0x203);
    func_8015DF18(0x202);
    D_1F800044[6] = 0x2;
    D_1F800044[1]++;
  }
}
