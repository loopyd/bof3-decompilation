#include "bof3/bof3.h"

extern u8 D_80144F28;

/* Scratchpad work-object pointer cell. Bound as a symbol in this target's map
 * at 0x1F800044 so each read re-materializes the base: the original loads the
 * cell once for the step halfword store and again after the flag probe, both
 * as `lui $v0,0x1F80` / `lw $v0,0x44($v0)`, and keeps no saved register for
 * the cell address. */
extern u8* g_world02Area11113Work; /* @source 0x1F800044 @kind data */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_80196070(void);
void func_801F3244(void);

/* @source 0x801F3130
 * @behavior First of the three byte-2 state handlers dispatched through the
 *           table based at 0x801F489C for the work record published at the
 *           scratchpad pointer cell 0x1F800044. It clears the record's
 *           unsigned halfword at offset 0x30 (the step counter the third
 *           handler advances by 4) and then probes flag 0x28 of the main-RAM
 *           bank D_80144F28 through the shared flag tester at 0x8015B5D4.
 *           When that flag reads set it resets the shared work area through
 *           the exe-resident helper at 0x80196070, so the record stays on
 *           this handler; when the flag reads clear it advances the record's
 *           dispatch byte at offset 0x02 and calls the overlay draw routine
 *           at 0x801F3244, which hands the next frame to handler 1. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps
 *           $ra at 0x10 across the calls, and the pre-call store of zero
 *           fills the delay slot of the flag probe.
 *
 * Name evidence: the cleared offset is the `sh $zero, 0x30($v0)` the original
 * issues in that delay slot; the flag is the `lui/addiu $a0, %hi(D_80144F28)`
 * plus `addiu $a1, $zero, 0x28` argument pair; the reset helper at 0x80196070
 * is the already-lifted clearWorkFlags body; the increment and draw pair is
 * the shape of the unlifted handler 1 at 0x801F319C, which tests the same
 * flag with the same index; and the dispatch byte 0x02 with the three
 * in-payload pointers 0x801F3130/0x801F319C/0x801F31F0 at 0x801F489C,
 * 0x801F48A0 and 0x801F48A4 make this entry 0 of that table.
 *
 * Shape: seeded from the original bytes after a payload scan found no
 * byte-identical twin (0 exact hits and 0 hits with every `j`/`jal` opcode
 * masked across out/binaries), so the clone and carve levers do not apply and
 * the range carries its own prologue and `jr $ra`. The cell spelling follows
 * the sibling at bmagic/magic104/03 0x801EEE7C, whose comment records the same
 * two-read cell: a scratchpad constant address shared by both reads was held
 * in a saved register across the call, while the symbol-bound cell reloads
 * the base for each read, which is what the original shows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void zeroWorkStepHalfword30ThenClearWorkFlagsOnFlag28ElseAdvanceHandlerByteWorld02Area11113_801F3130(
    void) {
  *(u16*)(g_world02Area11113Work + 0x30) = 0;
  if (func_8015B5D4((u32)&D_80144F28, 0x28) != 0) {
    func_80196070();
  } else {
    g_world02Area11113Work[2] = g_world02Area11113Work[2] + 1;
    func_801F3244();
  }
}
