#include "bof3/bof3.h"

extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */
extern u8 D_8014933B;  /* @source 0x8014933B @kind data */

/* @source 0x801F31BC @kind function */
extern void func_801F31BC(void);

/* @source 0x801F38F0
 * @behavior Entry 3 of the state-byte-2 handler run based at 0x801F4AB0 of
 *           this payload, whose dispatcher is 0x801F3764. Returns straight to
 *           the payload render helper 0x801F31BC while the shared overlay
 *           state byte D_8014933B holds 0x20. On every other value it reads
 *           the scratchpad work record through the pointer cell 0x1F800044
 *           once and advances work byte +2 at most once per run: the record
 *           halfword +0x36 and the shared camera word D_80145EC4 are compared
 *           first through the cell centre (((halfword + 1) << 16) | 0x8000),
 *           and only when that absolute difference stays within 0x20000 is the
 *           halfword +0x3A and the shared camera word D_80145EC8 compared the
 *           same way, once against the cell start (halfword << 16) and once
 *           against the next cell start ((halfword + 1) << 16). Calls the
 *           render helper 0x801F31BC on every path. Takes no arguments and
 *           returns nothing. The shared state byte and camera words are
 *           outside this payload.
 *
 *           Shape evidence: transcribed from the exact same-target body
 *           advanceRecordCoordinateWindow,
 *           taking that body's flat else arm for the statement order, the
 *           abs() expansion, the cell-centre operand form and the offset-local
 *           spelling, with this span's guard, its fixed 0x36-then-0x3A axis
 *           order and its own literals and call.
 *
 *           Measured variants: moving the window constant before the record
 *           pointer dereference, before the pointer address, and above the
 *           guard; inline 0x20000 literals in all three comparisons; a guard
 *           spelled as an early jump to a trailing draw label; a negated
 *           comparison; and three compositions of those neutral spellings. All
 *           kept the residual below, and the two that moved the constant above
 *           the guard or changed the comparison polarity regressed to 43 of 52.
 * @status partial
 * @match 96.08
 * @residual The body, the 0xCC size and every control-flow decision match. One
 *           two-word scheduling difference remains: the original materializes
 *           the 0x20000 window constant in the load-delay slot of the work
 *           record pointer load (word 8 of 51) and leaves the abs branch delay
 *           slot a nop, while this build commits the same constant to that
 *           branch delay slot (word 17) and leaves the pointer load slot a nop.
 *           Compiler RTL at the owning profile keeps the constant between the
 *           pointer load and the first halfword read through sched2, and the
 *           delay-branch reorganization then moves it into the branch delay
 *           slot. Nine clean-C variants covering statement order, literal form,
 *           jump and label control flow, comparison polarity and their
 *           compositions left it unchanged. A first-window predicate captured
 *           before the sign test kept the constant early but duplicated the
 *           comparison and regressed to 47 of 51; branch-local predicates
 *           returned to 49 of 51 with the original scheduling residual.
 *           The same-family sibling
 *           advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld04Area17113_801F2F5C
 *           carries the same review-pending residual in its own target. No
 *           compiler, flag or binding change was used.
 *
 *           Unsigned shifts, subtraction and negation preserve modulo-32-bit
 *           arithmetic for negative cell indices and overflowing differences.
 *           Conversion back to s32 uses the owning compiler's two's-complement
 *           interpretation, including a negative result for negated 0x80000000.
 */
void stateCameraOutsideOverlay(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  if (D_8014933B != 0x20) {
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

  func_801F31BC();
}
