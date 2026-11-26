#include "bof3/bof3.h"

/* Shared camera coordinate words of the main executable, published by the
 * front-end selectors and projected by the world overlays. Both lie outside
 * this overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* @source 0x801F2F5C
 * @behavior Overlay step gate over the scratchpad work record published at the
 *           pointer cell 0x1F800044, reached as word 3 of the five-word handler
 *           run based at 0x801F3F5C, whose other words are 0x801F2CA0,
 *           0x801F2E1C, 0x801F2F00 and 0x801F3000: the overlay dispatcher at
 *           0x801F2C5C calls the word the record state byte at offset 0x02
 *           selects, so no jal names this address. The gate takes the record
 *           halfword at 0x36 as a cell index, builds that cell's centre
 *           (((halfword + 1) << 16) | 0x8000), and advances the record state
 *           byte at 0x02 when its absolute difference from the shared camera
 *           word D_80145EC4 exceeds 0x20000; only when that check passes does
 *           it read the record halfword at 0x3A and the matching camera word
 *           D_80145EC8, advancing the same byte when the absolute difference
 *           from both that halfword's own cell start (halfword << 16) and the
 *           next cell's start ((halfword + 1) << 16) exceeds 0x20000. Unlike
 *           the family members named below it carries no route-byte dispatch:
 *           the 0x36 axis is always tried against D_80145EC4 first and the
 *           0x3A axis against D_80145EC8 in the tail. Takes no arguments,
 *           returns nothing, calls nothing and keeps no frame; the cell is
 *           loaded once and that one base serves the halfword reads and the
 *           byte store.
 *
 *           Shape evidence: this 0xA4 span was compared with the exact bodies
 *           of advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld04Area16913_801F2F68
 *           (0x110 bytes), advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld03Area14813_801F3020
 *           (0x110) and advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld04Area17013_801F3F84
 *           (0xFC), which are byte-different in more than relocations (their
 *           route-byte test at record offset 0x08 selects which axis and
 *           camera word lead, and area169 ends in its own payload draw call),
 *           so none of them is a clone of this span. This body transcribes
 *           their zero-route arm: the same statement order, abs() expansion,
 *           cell-centre operand form and offset-local spelling, with this
 *           span's own halfword offsets and no dispatch or tail call.
 * @status partial
 * @match 95.12
 * @residual One 2-word scheduling difference: the original materializes the
 *           0x20000 window constant (lui $a2, 0x2) in the entry block, where
 *           it fills the load-delay slot of the work-record pointer load (word
 *           3 of 41) and leaves a nop in the abs() bgez slot (word 12); this
 *           build sinks the same constant to its first use and commits it to
 *           that bgez slot instead, leaving the pointer load's slot a nop. Every
 *           other word, the 0xA4 size and all three abs()/slt chains agree.
 *           The inherited limit/order/control-flow probes and the modular
 *           arithmetic forms left this pair unresolved. Compiler RTL at the
 *           owning profile retains the constant near the pointer load through
 *           sched2; delay-branch reorganization moves it into the first bgez
 *           sequence. A guarded builtin abs form added branches and regressed.
 *           No compiler or flag change was used; the scheduling residual remains
 *           review-pending, not proven unreachable in clean C.
 *
 *           Unsigned shifts, subtraction and negation preserve the original
 *           modulo-32-bit arithmetic for negative cell indices and overflowing
 *           differences. Conversion back to s32 uses this target compiler's
 *           two's-complement interpretation, including the original negative
 *           result when negating 0x80000000.
 */
void advanceRecordCoordinateWindow(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  /* Reuse the consumed cell address for its loaded record pointer. Keeping
   * this real address-to-record chain preserves the original register web. */
  work = SPAD_ADDR(u8, 0x44u);
  work = *(u8**)work;
  limit = 0x20000;

  {
    s32 d;

    d = (s32)((u32)D_80145EC4 -
              ((((u32)FIELD_REF(s16, work, 0x36) + 1u) << 16) | 0x8000u));
    if (d < 0) {
      d = (s32)(0u - (u32)d);
    }
    d = d > limit;
    if (d) {
      goto advance;
    }
    value = FIELD_REF(s16, work, 0x3A);
    base = D_80145EC8;
  }

  {
    s32 d;

    d = (s32)((u32)base - ((u32)value << 16));
    if (d < 0) {
      d = (s32)(0u - (u32)d);
    }
    d = d > limit;
    if (d) {
      d = (s32)((u32)base - (((u32)value + 1u) << 16));
      if (d < 0) {
        d = (s32)(0u - (u32)d);
      }
      d = d > limit;
      if (d) {
advance:
        work[2]++;
      }
    }
  }
}
