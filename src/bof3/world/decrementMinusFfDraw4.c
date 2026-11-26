#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
/* @source 0x801F3050 */
extern void func_801F3050(void);

/* @source 0x801F2E64
 * @behavior Entry 2 of the state-byte +0x02 handler table based at 0x801F47C4,
 * the table indexed by the work byte at +0x02 of the record published at the
 * scratchpad pointer cell 0x1F800044: subtracts 0x20 from that record unsigned
 * halfword at offset 0x30 and stores the truncated halfword, and, once the
 * stored value read as a signed halfword falls below -0xFF, increments the
 * record byte at offset 0x02 so the next dispatch takes table entry 3 at
 * 0x801F2EC0. It then calls the payload record draw routine at 0x801F3050,
 * which imports SetDrawMode, SetPolyFT4, SetShadeTex and RotTransPers4. Takes
 * no arguments and returns nothing; the record pointer is loaded before the
 * frame is created, the 0x18-byte frame only keeps $ra at 0x10 across the
 * call, and the slots after the halfword load and after the branch stay nops.
 *
 * A word compare of this 0x5C span against the payloads measured the same
 * statement order in the World03/Area13913 body at 0x801F3DA8, which differs
 * in exactly one word, its jal at 0x801F3DEC to that overlay's own draw
 * routine 0x801F3F84; the spans are relocation-different rather than equal, so
 * that body supplied shape and this one's jal immediate, pointer-cell binding
 * and offset spelling come from this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMinusFfDraw(void) {
    u16 *work;
    u16 half;

    work = g_WorkRecordWorld03Area14013;
    half = work[0x18] - 0x20;
    work[0x18] = half;
    if ((s16)half < -0xFF) {
        ((u8 *)work)[2]++;
    }
    func_801F3050();
}
