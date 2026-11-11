#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. This target's
 * map binds the cell at 0x1F800044, and the original re-reads the cell on
 * either side of the sound-cue gate, so the named cell is rematerialised rather
 * than kept live across the call.
 * @source 0x1F800044 @kind data
 */
extern u8* area16813WorkCursor_1F800044;

/* Front-end phase byte of the shared main executable, outside this payload;
 * Splat binds it through the shared symbol map. The world04 sibling handlers
 * read it as the front-end phase that gates a cue while it is zero, and the
 * plchar lifts leave its role open, so the symbol stays address-anchored.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Sound-cue dispatcher of the shared main executable, outside this payload; the
 * exact sibling handlers pass it a cue id and discard any result.
 * @source 0x8015DF18 @kind function
 */
extern void func_8015DF18(u16 cue);

/* Update routine of this payload, still an `asm` span at 0x801F3914: it re-reads
 * the same work record, writes scratchpad and GTE state, and answers nothing.
 * @source 0x801F3914 @kind function
 */
extern void func_801F3914(void);

/* @source 0x801F38A4
 * @behavior Byte-2 state handler reached through the handler run based at
 *           0x801F4B2C of this payload: the body address appears once in the
 *           payload bytes, at word 0x801F4B98, which is entry 27 of that run,
 *           so no jal in this payload names it, it takes no arguments and it
 *           answers no value. The body reads the work record through the
 *           scratchpad pointer cell 0x1F800044, subtracts 0x10 from the
 *           record halfword at offset 0x30, stores the truncated halfword and
 *           tests the stored value as a signed halfword: while that value stays
 *           above zero the handler only runs the local helper 0x801F3914. Once
 *           the countdown reaches zero or below, the shared front-end phase
 *           byte D_80143BB0 gates the sound cue 0x201 through func_8015DF18,
 *           which fires only while that byte reads zero, and the record's state
 *           byte at offset 0x02 is set to 1 so the next dispatch takes entry 1
 *           of the same run. The pointer cell is loaded before the frame is
 *           created and re-read after the cue gate, both times as `lui` plus the
 *           offset load into $v1. The local helper 0x801F3914 runs on every
 *           call, including the frame that expires the countdown. The 0x18-byte
 *           frame only keeps $ra at 0x10, and the slots after the halfword load
 *           and after each branch stay nops. The roles of the 0x30 halfword, of
 *           the 0x2 state byte and of the requested cue stay address-anchored.
 *
 * Name evidence: a payload cmp of this 0x70-byte span against the published
 * exact lift at emi/world04/area170/13@0x801F4080 differs in exactly one word,
 * the absolute jump immediate at +0x58, which names 0x801F3914 here and
 * 0x801F40F0 there, so the two spans are one body and the sibling fixes the role
 * of the 0x30 countdown, of the 0x201 cue and of the D_80143BB0 phase gate. That
 * word follows each overlay's own address, so the sibling is shape evidence and
 * not clone eligibility. The `ThenCall3914` ending follows the two exact
 * neighbours of this target at 0x801F3738 and 0x801F3794, which name the same
 * tail helper by address.
 *
 * Shape evidence: the sibling at 0x801F3738 is the mirror of this handler - it
 * adds 0x10 to the same halfword and advances the state byte at 0x80 - and it
 * fixes this target's named-cell accessor, its halfword read-modify-write and
 * its empty delay slot before the tail call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1ThenCall3914World04Area16813_801F38A4(
    void) {
  u8* work;
  u16 half;

  work = area16813WorkCursor_1F800044;
  half = *(u16*)(work + 0x30) - 0x10;
  *(u16*)(work + 0x30) = half;
  if ((s16)half <= 0) {
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
    work = area16813WorkCursor_1F800044;
    work[2] = 1;
  }
  func_801F3914();
}
