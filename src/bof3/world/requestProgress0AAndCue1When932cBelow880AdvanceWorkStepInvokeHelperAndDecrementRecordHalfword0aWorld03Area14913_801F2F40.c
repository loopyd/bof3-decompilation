#include "bof3/bof3.h"

/* Shared signed halfword at 0x8014932C. Step 0 of this overlay's 0x801F4580
 * handler run compares it against 0x880, the neighbouring step handler
 * func_801F2FC4 against 0x680, and the already-exact world03/area121/14 lift
 * advanceHalfword932cTo1500World03Area12114_801F2CBC against 1500, all with a
 * signed halfword load.
 * @source 0x8014932C @kind data
 */
extern s16 D_8014932C;

/* Shared flag byte at 0x80147AD0. This step stores 1 into it and the next step
 * handler func_801F2FC4 stores 3, so the role stays address-anchored. Store
 * width comes from the original's single sb.
 * @source 0x80147AD0 @kind data
 */
extern u8 D_80147AD0;

/* Shared scenario progress byte; the overlay handlers publish the next progress
 * value here and this step's value is 0xA. Store width comes from the
 * original's single sb.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared record pointer cell at 0x80148210. Its +0x0A halfword is the countdown
 * every step of this script subtracts 2 from; the original loads the cell once
 * per step and stores through the loaded pointer.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Resident front-end cue dispatcher; this step queues cue 1. */
void func_8015DF18(u16 cue_id);

/* Local step helper at 0x801F3140. The four handlers of the 0x801F4580 run
 * call it with -4, -3, -2 and -1, so step 0 passes -4. */
void func_801F3140(s16 step_code);

/* @source 0x801F2F40
 * @behavior Step 0 of this overlay's four-entry handler run based at 0x801F4580,
 * which func_801F2EFC indexes with work byte 4. Reads the shared signed
 * halfword 0x8014932C once, before the frame is set up, and when it is below
 * 0x880 stores 1 into the shared flag byte 0x80147AD0, stores 0xA into the
 * scenario progress byte, queues front-end cue 1 through func_8015DF18, and
 * increments byte 4 of the scratchpad work record published at pointer cell
 * 0x1F800044 so the dispatcher advances to step 1. Whether or not that gate
 * fires it then calls the local step helper func_801F3140 with -4 and subtracts
 * 2 from the +0x0A halfword of the record published through 0x80148210. Takes
 * no arguments and returns nothing; the 0x18-byte frame keeps $ra at +0x10 and
 * the original's two unfilled load delay slots stay nops.
 *
 * Name evidence: the role comes from the payload's own dispatch tables. func_
 * 801F2EFC (0x801F2EFC) loads the scratch work record, scales its byte 4 by
 * four and calls the pointer at that index of the four-word table at
 * 0x801F4580, whose words are 0x801F2F40, 0x801F2FC4, 0x801F3068 and
 * 0x801F3108, so this body is step 0 of that run and its own byte-4 increment is
 * the step advance. Each of the four steps stores its own scenario progress
 * value and calls 0x801F3140 with its own negative code (-4 for step 0), so
 * both the progress store and the helper call are the run's shared idiom rather
 * than this step's identity. No in-image caller exists beyond the 0x801F4580
 * word; the overlay is reached through its dispatch tables.
 *
 * Shape evidence: func_801F2FC4 (0x801F2FC4) is the neighbouring step and
 * carries the same 0x18-byte frame, the same signed-halfword gate on 0x8014932C
 * read before the prologue, the same cue-then-advance byte-4 idiom, the same
 * -N call into 0x801F3140 and the same "subtract 2 from the +0x0A halfword"
 * tail, differing only in its own threshold (0x680), progress value (0xF), cue
 * argument (0), flag value (3) and the extra flag-gated store it adds, so its
 * statement order, pointer-cell spelling and record-offset arithmetic are
 * reused here with this step's own literals taken from the original bytes. Its
 * row is still asm, so it is shape evidence and not a clone; no payload in
 * out/binaries carries these 132 bytes elsewhere, so no byte-identical sibling
 * exists to clone from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestProgress0AAndCue1When932cBelow880AdvanceWorkStepInvokeHelperAndDecrementRecordHalfword0aWorld03Area14913_801F2F40(
    void) {
  s16 countdown;
  u8* work;

  countdown = D_8014932C;
  if (countdown < 0x880) {
    D_80147AD0 = 1;
    g_ScenarioProgress = 0xA;
    func_8015DF18(1u);
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[4] += 1;
  }
  func_801F3140(-4);
  D_80148210[5] -= 2;
}
