#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */
/* @source 0x801F3050 */
extern void func_801F3050(void);

/* @source 0x801F2EC0
 * @behavior Entry 3 of the twelve-entry state-byte +0x02 handler table based at
 *           0x801F47C4, whose neighbouring entries 0x801F2E64 and 0x801F2F7C
 *           were already lifted exactly when this seed was written. It reads the
 *           work record published at the scratchpad pointer cell 0x1F800044 and
 *           takes the signed record halfwords at +0x36 and +0x3A as the
 *           whole-cell parts of the shared camera coordinates 0x80145EC4 and
 *           0x80145EC8. The record state byte at +0x02, the byte the dispatcher
 *           indexes, advances by one when the camera word 0x80145EC4 lies more
 *           than 0x20000 away from the cell centre
 *           (((halfword at +0x36) + 1) << 16 | 0x8000), or when 0x80145EC8 lies
 *           more than 0x20000 below the record row start
 *           ((halfword at +0x3A) << 16) and more than 0x20000 beyond the next
 *           row start (((halfword at +0x3A) + 1) << 16); every other path keeps
 *           the byte, so the next dispatch takes entry 4 at 0x801F2F7C, which
 *           sets that byte back to 1. Each guard takes an absolute difference
 *           through the integer absolute-value intrinsic, and the 0x20000 limit
 *           is materialised once, in a register the compiler keeps across all
 *           three guards, before the saved register is stored. Every path ends
 *           by calling the payload draw routine at 0x801F3050, the only
 *           reference to it from this body. Takes no arguments and returns
 *           nothing; the 0x18-byte frame keeps $ra at 0x10, the scratchpad cell
 *           load follows the frame creation, and no payload word names this
 *           span, so the table entry reaches it and its roles stay structural.
 *
 *           Shape evidence: a little-endian word compare of this 0xBC span
 *           against every payload under out/binaries matched no span within
 *           eight words of this one, so clone was declined. The 41 words from
 *           +0x14 to the end of the span are nevertheless word-identical to the
 *           matching region of larger route-byte-gated handlers the same overlay
 *           family ships: the block starting at 0x801F3E58 of World03/Area13913
 *           and the block starting at 0x801F2FDC of World04/Area15713 through
 *           Area16913 and Area17413, each differing from this span in the five
 *           words before that region and in the draw `jal` at +0xA4. Those
 *           bodies prove this statement order and the 0x20000 limits, but they
 *           are relocation-different near-identical siblings and not clone
 *           proof, so the record pointer binding, the offset spelling and the
 *           constants were written from this span's own bytes, and the
 *           pointer-cell and draw declarations follow the exact handlers of this
 *           target.
 *
 *           Representation notes: a goto-shaped chain of the same three guards,
 *           its negation spelled as a signed test on a local, measured 25/47 with
 *           184 bytes, because the compiler if-converted each negation into a
 *           delay-slot compare and came out one instruction short. The same chain
 *           with the intrinsic on a local restored the count to 188 bytes but
 *           measured 26/47, placing the record pointer in $a1 instead of $a0;
 *           re-reading the cell and the camera word instead of binding them to
 *           locals changed nothing measurable. Nesting the guards as an if/else
 *           chain with the increment written twice measured 31/47. Keeping the
 *           intrinsic inside the guard while holding the difference in a local
 *           measured 42/47, first differing at +0x2C. The exact form above inlines
 *           the intrinsic in each guard with no intermediate difference variable.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCellWindowDraw(void) {
    u16 *work;

    work = g_WorkRecordWorld03Area14013;
    if (__builtin_abs(D_80145EC4 - ((((s32)(s16)work[0x1B] + 1) << 16) | 0x8000)) > 0x20000) {
        ((u8 *)work)[2]++;
    } else if (__builtin_abs(D_80145EC8 - ((s32)(s16)work[0x1D] << 16)) > 0x20000) {
        if (__builtin_abs(D_80145EC8 - (((s32)(s16)work[0x1D] + 1) << 16)) > 0x20000) {
            ((u8 *)work)[2]++;
        }
    }
    func_801F3050();
}
