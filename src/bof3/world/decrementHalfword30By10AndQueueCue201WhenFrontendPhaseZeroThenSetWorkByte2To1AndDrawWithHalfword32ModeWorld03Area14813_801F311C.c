#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front-end selectors and
 * re-read by the handlers of this overlay. The exact sibling at 0x801F2FA4 of
 * this target names it the same way; its address-to-record load precedes the
 * frame creation in both bodies, so the cell stays a real pointer object rather
 * than a fixed-address accessor. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Front-end phase byte of the main executable, outside this overlay payload;
 * Splat binds it through this target generated undefined-syms map. The exact
 * cue gates at emi/world04/area170/13 0x801F4080 and emi/world04/area165/13
 * 0x801F3518 read it as the front-end phase, so the symbol stays
 * address-anchored. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable, outside this overlay payload. */
void func_8015DF18(u16 arg0);

/* Overlay draw routine of this payload, still an `asm` span of this target at
 * 0x801F31AC. Its one argument is a signed byte mode: the routine consumes the
 * low byte only. */
void func_801F31AC(s8 arg0);

/* @source 0x801F311C
 * @behavior Entry 4 of the five-entry handler table based at 0x801F4FC4, the
 *           table the overlay dispatcher at 0x801F2C5C indexes by the state
 *           byte at offset 0x02 of the work record published at the scratchpad
 *           pointer cell 0x1F800044, so it has no jal caller. Subtracts 0x10
 *           from the record halfword at offset 0x30, stores the truncated
 *           halfword and tests the stored value as a signed halfword: while
 *           that value stays above zero the handler only draws. Once the
 *           countdown reaches zero or below, the front-end phase byte
 *           D_80143BB0 gates the sound cue 0x201 through func_8015DF18, which
 *           fires only while that byte is zero, and the record state byte at
 *           offset 0x02 is set to 1 so the next dispatch leaves this entry. The
 *           draw then re-reads the pointer cell, reads the signed record
 *           halfword at offset 0x32 and calls the overlay draw routine at
 *           0x801F31AC with mode -1 when that halfword is zero and mode 0 when
 *           it is not, including on the frame that expires the countdown. The
 *           pointer cell is loaded before the frame is created and is re-read
 *           after the cue gate and again before the draw, each time as lui plus
 *           the offset load. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps ra at 0x10.
 *
 * Name evidence: the same countdown body lifted exact in emi/world04/area170/13
 * at 0x801F4080 fixes the roles of the 0x30 countdown, the 0x201 cue and the
 * D_80143BB0 phase gate, and its statement order carries over unchanged. The
 * state action differs there, so this clause follows the SetWorkByte2To1
 * spelling of that exact lift, while the trailing draw follows the
 * ...ThenDrawWithHalfword32Mode ending of this target own exact 0x801F2FA4
 * handler, which selects its mode from the same record halfword at offset
 * 0x32.
 *
 * Shape evidence: a masked little-endian scan of every payload under
 * out/binaries, with relative branch words masked, finds this 144-byte body
 * here and at 0x801F30AC of emi/world03/area147/13, where the single differing
 * word is the jal to its own draw routine at 0x801F313C; that twin is still an
 * asm row there, so it supplies shape only and no clone. The head is
 * word-identical to the exact world04/area170 handler at 0x801F4080, whose
 * tail is a bare draw call instead, and the mode-selecting tail is
 * word-identical to the exact same-target handler at 0x801F2FA4, whose head
 * increments the halfword instead of decrementing it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1AndDrawWithHalfword32ModeWorld03Area14813_801F311C(
    void) {
  u8* work;
  u8* emitter;
  u16 half;

  work = D_1F800044;
  half = *(u16*)(work + 0x30) - 0x10;
  *(u16*)(work + 0x30) = half;
  if ((s16)half <= 0) {
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
    work = D_1F800044;
    work[2] = 1;
  }
  emitter = D_1F800044;
  if (*(s16*)(emitter + 0x32) != 0) {
    func_801F31AC(0);
  } else {
    func_801F31AC(-1);
  }
}
