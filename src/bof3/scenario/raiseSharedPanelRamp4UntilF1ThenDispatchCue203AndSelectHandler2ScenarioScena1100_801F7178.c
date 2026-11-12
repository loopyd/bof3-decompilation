#include "bof3/bof3.h"

/* Shared signed panel ramp halfword at 0x801492DA, the middle of the three
 * shared panel coordinates D_801492D8 / D_801492DA / D_801492DC that the sibling
 * at 0x801F7104 reseeds the 16.16 panel accumulators from and that the shared
 * panel update writes its integer part back into. Bound by
 * config/targets/shared/symbols.txt. */
extern s16 D_801492DA;

/* Shared primary mode byte at 0x80143BB0. Sibling scenario controllers request
 * primary mode 2 by storing 2 there and wait for it to leave 2 again, so a zero
 * here means no requested mode change is outstanding. */
extern u8 D_80143BB0;

/* Resident helper outside this payload at 0x80154FD8; it consumes the ramp
 * value this handler keeps handing it and returns nothing. */
/* @source 0x80154FD8 @kind unknown */
extern void func_80154FD8(u32 value);

/* Resident executable sound-cue dispatcher at 0x8015DF18; it consumes one cue
 * id and returns nothing. Named after the exact lift dispatchSoundCue
 * (0x8015DF18) of exe/slus_004_22. */
/* @source 0x8015DF18 @kind unknown */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F7178
 * @behavior Handler 1 of this overlay's handler run at 0x801FAEC0, which the
 *           dispatcher at 0x801F70B4 reaches with the work-record byte at
 *           offset 1 and calls with no arguments. The handler reads the shared
 *           panel ramp halfword D_801492DA (0x801492DA) unsigned, stores that
 *           ramp raised by four back into the same cell, and hands the previous
 *           ramp plus 0xF4 - the stored value plus 0xF0 - sign-extended to the
 *           resident helper func_80154FD8 (0x80154FD8), the same
 *           commit-then-pass shape its mirror handler at 0x801F7200 uses in the
 *           other direction with -0x14 and -4. Once the re-read signed ramp
 *           reaches 0xF1 it dispatches cue 0x203 through the resident sound-cue
 *           dispatcher 0x8015DF18 when the shared primary mode byte D_80143BB0
 *           is zero, then publishes 2 in work-record byte 1 through the
 *           scratchpad pointer cell 0x1F800044, so the next dispatch of the run
 *           selects its third word (0x801F7200), the mirror that lowers the
 *           ramp back. Takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps $ra at 0x14 and the ramp cell address at $s0
 *           across the resident call. The only in-image reference to
 *           0x801F7178 is the word at 0x801FAEC4, the second of the run's three
 *           leading code words (0x801F7104, 0x801F7178, 0x801F7200), so the
 *           dispatcher enters it with work byte 1 and the overlay loader never
 *           names it directly.
 *
 * Shape notes from the original bytes. The ramp cell address is materialised
 * once with `lui`/`addiu` into $s0 and kept across the call, so both the
 * unsigned `lhu` at 0x801F718C and the post-call signed `lh` at 0x801F71AC read
 * through displacement 0. The original stores the raised value in the `jal`
 * delay slot, and the raised value is computed from the same pre-store load, so
 * the argument is the previous ramp plus 0xF4 while the stored value is that
 * ramp plus four. The mode byte test at 0x801F71CC branches past the call when
 * the byte is nonzero, so the cue call sits on the zero side, and the pointer
 * cell at 0x1F800044 expands to `lui` plus an offset load read once after that
 * call. No exact sibling carries this body: a search of the extracted assembly
 * of every target found the discriminating `addiu $a0, $v0, 0xF4` only in this
 * span, and the mirror handler 0x801F7200 (still on its `asm` row) differs in
 * its literals, its store direction and its branch polarity, so that disassembly
 * supplies shape only and not clone eligibility. The body was seeded from the
 * original instruction words above.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseSharedPanelRamp4UntilF1ThenDispatchCue203AndSelectHandler2ScenarioScena1100_801F7178(void) {
  u16 ramp;
  s16 *cell;
  u8 *work;

  cell = &D_801492DA;
  ramp = *(u16 *)cell;
  *cell = ramp + 4;
  func_80154FD8((s16)(ramp + 0xF4));
  if (*cell < 0xF1) {
    return;
  }
  if (D_80143BB0 == 0) {
    dispatchSoundCue(0x203);
  }
  work = SPAD_PTR_SLOT(u8, 0x44u);
  work[1] = 2;
}
