#include "bof3/bof3.h"

/* The overlay step routine this handler runs once the record halfword has
 * climbed back to zero. It is a neighbouring code span of this overlay and is
 * still lifted from assembly, so only its own prototype is declared here. */
void func_801F33A4(void);

/* The overlay record draw routine every reaching path of this step finishes
 * with, called here with a zero mode argument. */
void func_801F381C(s32 arg0);

/* Front-end phase byte of the main executable, the same gate the overlay step
 * routine at 0x801F33A4 and the neighbouring handlers read. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Work record pointer cell of the scratchpad, addressed as its own absolute
 * symbol so each of the three reads below rematerialises the cell address from
 * one lui/offset-load pair, as the original does. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* @source 0x801F36B4
 * @behavior Byte-2 state handler that both byte-2 dispatch tables of this
 *           overlay list at entry 4 (0x801F516C, indexed by the dispatcher at
 *           0x801F3014, and 0x801F5184, indexed by the dispatcher at
 *           0x801F3C10); either table indexes the work record published at the
 *           scratchpad pointer cell 0x1F800044 by its state byte at offset
 *           0x02. It is the second half of the two-step halfword slide whose
 *           first half is 0x801F3534: it adds 0x20 to that record's unsigned
 *           halfword at offset 0x30 and stores the truncated halfword. While
 *           the stored value read as a signed halfword is still negative the
 *           step only draws, so the record stays in state 4; the draw routine
 *           at 0x801F381C reads the same halfword as a sprite screen offset.
 *           Once that value is no longer negative, the handler calls the
 *           overlay step routine at 0x801F33A4, which walks the record against
 *           the shared camera words and can put the record back into state 2
 *           while it draws with mode 1; when it did, this handler returns
 *           without drawing again. Otherwise, while the front-end phase byte
 *           0x80143BB0 reads zero, it plays sound cue 0x201, then stores 1 into
 *           the record state byte at offset 0x02 so a later dispatch takes a
 *           steady-state entry of the same table. Every path that reaches the
 *           draw calls 0x801F381C with a zero mode argument. Takes no
 *           arguments and returns nothing; the record pointer is loaded before
 *           the frame is created, the 0x18-byte frame only keeps $ra at 0x10
 *           across the calls, the slots after the halfword load and after each
 *           branch stay nops, and the pointer cell is reloaded after both
 *           calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementHalfword30By20AndSetWorkByte2To1WhenNonNegativeThenDrawWorld03Area11813_801F36B4(
    void) {
  u8* work;
  u16 half;

  work = D_1F800044;
  half = *(u16*)(work + 0x30) + 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half >= 0) {
    func_801F33A4();
    if (D_1F800044[2] == 2) {
      return;
    }
    if (D_80143BB0 == 0) {
      dispatchSoundCue(0x201);
    }
    D_1F800044[2] = 1;
  }
  func_801F381C(0);
}
