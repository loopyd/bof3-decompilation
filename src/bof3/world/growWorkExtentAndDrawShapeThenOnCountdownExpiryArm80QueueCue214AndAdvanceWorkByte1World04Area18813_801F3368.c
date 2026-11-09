#include "bof3/bof3.h"

/* Overlay quad renderer of this area. It projects the record's three-word point
 * handed as arg0 through the shared point projector func_801AFF04, passes the
 * signed extent handed as arg1 as a size pair to the shared size-pair converter
 * func_801AFFD8, and draws one semi-transparent four-vertex primitive through
 * g_PrimCursor whose colour bytes come from the channel mask handed as arg2 -
 * bit 2 selects the red byte 0x80, bit 1 the green byte 0x80 and bit 0 the blue
 * byte 0x80. */
void func_801F38E0(void* point, s16 extent, s32 channels);

/* Shared EXE-side cue dispatcher; the constant argument seeds a caller's
 * channel refresh, so the call-site width is the narrow constant form. */
void func_8015DF18(u16 arg0);

/* @source 0x801F3368
 * @behavior Overlay step handler, entry 1 of the state-byte-1 handler table at
 *           0x801F4E04 that the dispatcher 0x801F32C0 indexes: it grows the
 *           extent halfword at +0x2E of the work record published at the
 *           scratchpad pointer cell 0x1F800044 by 0x20, stores it back and
 *           renders the record through the overlay renderer 0x801F38E0 with
 *           the record's three-word point at +0x64, the sign-extended extent
 *           and the channel mask 0x6, so the shape drawn at that point widens
 *           every frame it runs. It then decrements the record's countdown
 *           byte at +0x09 and, on the frame that byte reaches zero, re-arms it
 *           with 0x80, advances the state byte at +0x01 and queues the
 *           front-end cue 0x214 through func_8015DF18. The extent halfword is
 *           read, advanced and stored through one pointer materialisation
 *           because nothing between them calls out, while the countdown, the
 *           re-arm, the state advance and the cue each re-read the scratchpad
 *           cell after a store through the record. It takes no arguments,
 *           returns nothing and reads no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void growWorkExtentAndDrawShapeThenOnCountdownExpiryArm80QueueCue214AndAdvanceWorkByte1World04Area18813_801F3368(
    void) {
  u8** slots;
  u8* work;
  u16 extent;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  extent = *(u16*)(work + 0x2E) + 0x20;
  *(u16*)(work + 0x2E) = extent;
  func_801F38E0(work + 0x64, (s16)extent, 6);
  if (--slots[0x11][9] == 0) {
    slots[0x11][9] = 0x80;
    slots[0x11][1]++;
    func_8015DF18(0x214);
  }
}
