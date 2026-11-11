#include "bof3/bof3.h"

/* Shared mode byte of the resident executable, outside this overlay image. The
 * reviewed World lifts index their own halfword tables with the same cell after
 * a signed byte read, and this body loads it with `lb` and scales it by two, so
 * it stays s8 here.
 * @source 0x801490C7 @kind data
 */
extern s8 D_801490C7;

/* Shared status halfword of the resident executable. The reviewed
 * `setStatusFFFF...` World lifts publish into it with `ori`/`sh`, which is the
 * store this body makes, so it stays u16.
 * @source 0x801490A8 @kind data
 */
extern u16 D_801490A8;

/* Mode-indexed halfword status table inside the payload's 0x801F4624 tail blob.
 * The body adds the signed mode scaled by two to this base and reads one `lhu`,
 * so the element is a halfword. Its halfwords read 0xFFFF for modes 0-4 and
 * 6-10, 0x002C at mode 5 and 0x002B at mode 11; the last four payload bytes
 * 0x22/0x22/0x07/0x85 follow at +0x18 and this body never reads them.
 * @source 0x801F55D8 @kind table
 */
extern u16 D_801F55D8[];

/* Shared scenario progress byte 0x80146864, bound by the shared symbol map as
 * g_ScenarioProgress. The World lifts store single-byte progress values into it.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared frontend cue dispatcher of the resident executable; the cue id arrives
 * zero-extended in the whole argument word, as this family's lifts record.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u16 cue);

/* @source 0x801F3FF0
 * @behavior Overlay mode handler: loads the shared mode byte 0x801490C7 with a
 *           signed byte read (`lb`), scales it by two (`sll $v0,$v1,1`) and
 *           indexes the overlay's halfword status table at 0x801F55D8 (`addu`
 *           into $at plus one `lhu`), publishing the selected halfword into the
 *           shared status halfword 0x801490A8 with `sh`. Only mode 5 (0x002C)
 *           and mode 11 (0x002B) publish a status; every other mode publishes
 *           0xFFFF. Modes 0 through 4 (`bltz` on the mode, `slti ...,5` in its
 *           delay slot, then `beqz`) also store 0x0A into the shared scenario
 *           progress byte g_ScenarioProgress 0x80146864 and request frontend cue
 *           0x205 through the resident frontend cue dispatcher 0x8015DF18
 *           (`addiu a0,zero,0x205` in the `jal` delay slot); a negative mode
 *           and any mode above 4 leave progress and cue untouched. Takes no
 *           arguments and returns nothing, and the 0x18-byte frame keeps only
 *           $ra at +0x10, which is why the mode load and the table read precede
 *           the frame reservation in the original schedule.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: statement order, operand forms and
 * declaration style are transcribed from two reviewed sibling sources of other
 * overlays that carry the same cells - 0x801F3C94 keeps this body's exact order
 * `mode = D_801490C7; D_801490A8 = <table>[mode];` followed by the mode-selected
 * progress store, and 0x801F4790 carries the same `func_8015DF18(cue)` arm with
 * g_ScenarioProgress. This body's own words are its table base 0x801F55D8, its
 * cue id 0x205, its progress constant 0x0A and its two-sided signed range guard,
 * so both siblings are shape evidence only. A payload scan for the whole 0x58
 * span found exactly one occurrence across the 396 shipped payloads, and a
 * masked scan that wildcards this span's absolute address words, its `jal` word
 * and its three constants also found no second body, so the clone lever had no
 * byte-identical source and was declined. The word at 0x801F5580 holds
 * 0x801F3FF0, so this span is entry 5 of the eight-word code-pointer run at
 * 0x801F556C, the run the pointer word at 0x801F55D0 also names.
 *
 * Match note: the range guard is written as two nested tests because the
 * original carries two signed compares - `bltz` on the mode and `slti
 * $v0,$v1,5` in its delay slot. The same guard spelled as one `&&` chain was
 * folded to a single unsigned `sltiu $v0,$v0,5` over a 21-instruction body and
 * measured 13/22 instructions and 88 -> 84 bytes; the nested spelling restores
 * the second compare, which also moves the live mode value to $v1 as the
 * original holds it, and measured 22/22 instructions at 88 -> 88 bytes.
 */
void publishModeStatusFromTable55d8ThenProgressAAndCue205ForModes0To4World03Area11713_801F3FF0(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F55D8[mode];
  if (mode >= 0) {
    if (mode < 5) {
      g_ScenarioProgress = 0xA;
      func_8015DF18(0x205);
    }
  }
}
