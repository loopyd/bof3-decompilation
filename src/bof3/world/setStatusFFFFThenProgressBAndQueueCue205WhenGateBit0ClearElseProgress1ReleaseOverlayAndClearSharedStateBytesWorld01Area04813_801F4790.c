#include "bof3/bof3.h"

/* Shared gate byte of the resident executable, outside this overlay image. The
 * reviewed World lifts read the same cell as the area entry gate: 0x801F381C
 * indexes its target-local table 0x801F4D64 with it and 0x801F2CB8 switches on
 * its value. This body only tests bit 0 after an `lbu`, so it stays u8.
 * @source 0x801490C7 @kind data
 */
extern u8 D_801490C7;

/* Shared status halfword of the resident executable. The reviewed
 * `setStatusFFFF...` World lifts publish 0xFFFF into it with `ori`/`sh`, which
 * is exactly the store this body makes, so it stays u16.
 * @source 0x801490A8 @kind data
 */
extern u16 D_801490A8;

/* Shared scenario progress byte 0x80146864, bound by the shared symbol map as
 * g_ScenarioProgress. The World and SCENA00 lifts store single-byte progress
 * values into it. */
extern u8 g_ScenarioProgress;

/* Primary state byte of the shared main-RAM state pair, outside this overlay
 * image. The reviewed lift 0x801F3884 stores the primary state here and the
 * substate in the byte above it; this body only clears both with `sb`, so they
 * stay unsigned bytes.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Substate byte of the shared main-RAM state pair; see the primary state byte
 * above.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared overlay release helper of the resident executable. The SCENA00 lifts
 * run it as a route step completes, and this body runs it on the same path that
 * clears the shared primary state pair.
 * @source 0x8015C058
 */
extern void func_8015C058(void);

/* Shared frontend cue dispatcher of the resident executable; the cue id arrives
 * zero-extended in the whole argument word, as this target's queueFrontCue20C
 * lift records.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u16 cue);

/* @source 0x801F4790
 * @behavior Publishes 0xFFFF into the shared status halfword 0x801490A8 with
 *   `ori`/`sh`, then tests bit 0 of the shared gate byte 0x801490C7, which it
 *   loads unsigned with `lbu` and masks with `andi 0x1`. While that bit reads
 *   clear it stores 0xB into the shared scenario progress byte g_ScenarioProgress
 *   and requests frontend cue 0x205 through the shared frontend cue dispatcher
 *   0x8015DF18; while the bit reads set it stores 1 into g_ScenarioProgress, runs
 *   the shared overlay release helper 0x8015C058 and clears the shared primary
 *   state byte 0x80146874 and its substate byte 0x80146875 in that order. Takes
 *   no arguments and returns nothing; the 0x18-byte frame keeps only $ra at
 *   +0x10 above the outgoing argument word, and the arm constants fill the
 *   `bnez` delay slot and the dispatcher call delay slot.
 *
 * Name evidence: the only word of this payload that names 0x801F4790 is entry 0
 *   of the eight-word handler run at payload offset 0x3798 (0x801F6398), whose
 *   later entries are this target's own 0x801F4804, 0x801F4864, 0x801F48DC,
 *   0x801F49C8, 0x801F4AB4 and 0x801F4B40 and the run base itself is published
 *   by the pointer word at 0x801F63E8. No `jal` reaches the span, so the
 *   caller is the table and the argument roles stay structural. Entry 0 of a
 *   handler run carries no role of its own; the name states what this body
 *   does, and the run membership is recorded here instead.
 *
 * Cell roles come from reviewed lifts, not from this span: 0x801F381C and
 *   0x801F2CB8 for the gate byte and the status halfword, 0x801F3884 for the
 *   shared primary state and substate pair, and this target's own queueFrontCue20C
 *   lift for the cue dispatcher prototype.
 *
 * Shape and lever evidence: the span is a function, not data and not a
 *   fragment - it opens with `addiu $sp,$sp,-0x18`, spills $ra and ends in
 *   `jr $ra` - so no data or carve lever applies. Clone is declined on payload
 *   evidence: a scan of all 396 extracted payloads under out/binaries for this
 *   exact 116-byte body returned only its own offset 0x1B90, and a second scan
 *   masking every absolute word (the two `lui`/low-half pairs, both `jal`
 *   words and the `j` word) also returned only that single hit, so no
 *   relocation-different sibling of this body exists to seed from. The
 *   statement order, the store order of the two shared state bytes and every
 *   constant are therefore written from these original bytes, and the branch
 *   direction is this span s own: `andi`/`bnez` over the gate bit selects the
 *   progress 1 arm, which matches the reviewed emission where `if (x & 1)`
 *   over a byte lands the then-arm in the fall-through slot only after `beqz`,
 *   so the seed writes the clear arm as the tested one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFThenProgressBAndQueueCue205WhenGateBit0ClearElseProgress1ReleaseOverlayAndClearSharedStateBytesWorld01Area04813_801F4790(
    void) {
  D_801490A8 = 0xFFFF;
  if ((D_801490C7 & 1) == 0) {
    g_ScenarioProgress = 0xB;
    func_8015DF18(0x205);
  } else {
    g_ScenarioProgress = 1;
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
  }
}
