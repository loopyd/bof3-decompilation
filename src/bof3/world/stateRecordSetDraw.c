#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
/* @source 0x801F47F4 @kind data */
extern u8 D_801F47F4[];
/* @source 0x801F3950 */
extern void func_801F3950(s32 arg0, s32 arg1);

/* @source 0x801F3320
 * @behavior Entry 0 of the handler run based at 0x801F47DC, the table this
 *           target's exact dispatcher
 *           dispatchStateByte2Table47 indexes with
 *           the work record state byte at +0x02, so no jal reaches this body.
 *           It reads the scratchpad work pointer published at 0x1F800044 and
 *           takes the signed halfword at +0x36 as an index scaled by four into
 *           the four-byte record run at 0x801F47F4. Record byte 0 and byte 1
 *           are stored as the record halfwords at +0x36 and +0x3A, record byte
 *           2 into the record byte at +0x08, and record byte 3 into the record
 *           byte at +0x0B after the pointer cell is re-read for that store.
 *           Re-reading the cell again, when the byte at +0x08 is zero it stores
 *           0 into the halfword at +0x3E and the byte at +0x02 with that same
 *           byte plus one, and otherwise stores 0x80 into the halfword at +0x3E
 *           and 4 into the byte at +0x02, so the dispatch index it leaves is
 *           either the next handler in that run or the selector 4. It re-reads
 *           the pointer cell a fourth time and calls the overlay draw routine
 *           at 0x801F3950 with the record byte at +0x08 and the signed halfword
 *           at +0x3E. Takes no arguments and returns nothing; the 0x18-byte
 *           frame holds ra at +0x10 and the first pointer load precedes the
 *           frame creation.
 *
 *           Name evidence: the span reads a scaled record from the local run at
 *           0x801F47F4 and installs it over the four record fields at +0x36,
 *           +0x3A, +0x08 and +0x0B, then selects the dispatch byte at +0x02 and
 *           the ramp halfword at +0x3E; the halfword pair and the +0x3E ramp
 *           are the fields the neighbouring exact handlers
 *           stateStepHalfwordDraw
 *           and stepWorkHalfword3World03 own, and the byte
 *           at +0x0B is the flag-bit index the exact handler
 *           stateFlagBitSubmit
 *           reads. No payload word names a role for the record run, so it stays
 *           address-anchored, and the record byte at +0x08 is passed to the
 *           draw routine only, not named.
 *
 *           Shape evidence: handler
 *           stepWorkHalfword3World03, entry 5 of the same
 *           run, was lifted to
 *           src/bof3/world/stepWorkHalfword3World03.c and
 *           measured 100.00 when this seed was written; its pointer-cell
 *           reload spelling and its draw tail through the 0x801F3950 call with
 *           the record byte at +0x08 and the signed halfword at +0x3E supplied
 *           that part. The four reload points and the table walk were written
 *           from this span's original bytes. A little-endian scan of every
 *           payload under out/binaries with the three relocation words (two j
 *           immediates and the bnez immediate) masked and the remaining 52
 *           words compared finds this 0xDC-byte body only at 0x801F3320 of its
 *           own payload, so clone was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateRecordSetDraw(void) {
    u16 *work;
    s32 scale;
    u16 *stateWork;
    u16 *tailWork;
    u8 state;

    work = g_WorkRecordWorld03Area14013;
    scale = (s16)work[0x1B] * 4;
    work[0x1B] = D_801F47F4[scale];
    work[0x1D] = D_801F47F4[scale + 1];
    ((u8 *)work)[8] = D_801F47F4[scale + 2];
    ((u8 *)g_WorkRecordWorld03Area14013)[0xB] = D_801F47F4[scale + 3];
    stateWork = g_WorkRecordWorld03Area14013;
    if (((u8 *)stateWork)[8] == 0) {
        state = ((u8 *)stateWork)[2] + 1;
        stateWork[0x1F] = 0;
    } else {
        stateWork[0x1F] = 0x80;
        state = 4;
    }
    ((u8 *)stateWork)[2] = state;
    tailWork = g_WorkRecordWorld03Area14013;
    func_801F3950(((u8 *)tailWork)[8], (s16)tailWork[0x1F]);
}
