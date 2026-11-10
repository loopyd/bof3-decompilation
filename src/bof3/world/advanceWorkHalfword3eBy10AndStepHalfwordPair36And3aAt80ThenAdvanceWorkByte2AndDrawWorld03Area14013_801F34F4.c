#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u16 *g_WorkRecordWorld03Area14013;
/* @source 0x8015507C */
extern void func_8015507C(s32 arg0, s32 arg1);
/* @source 0x80155218 */
extern void func_80155218(s32 arg0, s32 arg1);
/* @source 0x801F3950 */
extern void func_801F3950(s32 arg0, s32 arg1);

/* @source 0x801F34F4
 * @behavior Overlay step handler: entry 8 of the handler run based at
 *           0x801F47C4, the table this target's exact dispatcher
 *           dispatchStateByte2Table47c4World03Area14013_801F2C34 indexes with
 *           the work record's state byte at +0x02, so no jal reaches this
 *           body. It reads the scratchpad work pointer published at 0x1F800044
 *           once, adds 0x10 to the record halfword at +0x3E, stores the
 *           truncated halfword and tests that stored value as a signed
 *           halfword against 0x80. When the value has reached 0x80 it calls
 *           0x8015507C with the signed halfwords at +0x36 and +0x3A, re-reads
 *           the pointer cell, calls 0x80155218 with both of those halfwords
 *           incremented by one and re-truncated to signed halves, then re-reads
 *           the pointer cell a third time and increments the state byte at
 *           +0x02 by one. On every path it re-reads the pointer cell and calls
 *           the overlay draw routine at 0x801F3950 with the record byte at
 *           +0x08 and the signed halfword at +0x3E. Takes no arguments and
 *           returns nothing; the 0x18-byte frame holds ra at +0x10 and the
 *           first pointer load precedes the frame creation.
 *
 *           The state byte reachable at +0x02 is the advance this body makes:
 *           the following entry, func_801F35B0 at 0x801F35B0, is the handler
 *           the next frame dispatches.
 *
 *           Name evidence: the ramp this body owns is the +0x3E halfword, the
 *           pair it hands to 0x8015507C and then steps by one is +0x36/+0x3A,
 *           and the store it makes to the dispatch index is the +0x02 byte, so
 *           all three fields stay address-anchored instead of asserting an
 *           unproven role.
 *
 *           Shape evidence: handler func_801F38C4 at 0x801F38C4, entry 11 of
 *           the same run, was lifted to
 *           src/bof3/world/stepWorkHalfword3eWorld03Area14013_801F38C4.c with
 *           a measurement of 100.00, and its draw tail from the reload of the
 *           pointer cell through the 0x801F3950 call is byte-identical here,
 *           so that source supplied the statement order, the pointer-local
 *           spelling and the tail. Handler func_801F37C4 at 0x801F37C4, entry
 *           10, supplied the in-block call pair, the incremented
 *           re-truncated argument form and the state-byte increment. A
 *           little-endian masked scan of every payload under out/binaries with
 *           the jal target words masked finds this 0xBC-byte body only at
 *           0x801F34F4 of this payload, so no byte-identical sibling exists and
 *           clone was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHalfword3eBy10AndStepHalfwordPair36And3aAt80ThenAdvanceWorkByte2AndDrawWorld03Area14013_801F34F4(void) {
    u16 *work;
    u16 *tailWork;

    work = g_WorkRecordWorld03Area14013;
    work[0x1F] += 16;
    if ((s16)work[0x1F] >= 0x80) {
        func_8015507C((s16)work[0x1B], (s16)work[0x1D]);
        func_80155218((s16)(g_WorkRecordWorld03Area14013[0x1B] + 1),
                      (s16)(g_WorkRecordWorld03Area14013[0x1D] + 1));
        ((u8 *)g_WorkRecordWorld03Area14013)[2] += 1;
    }
    tailWork = g_WorkRecordWorld03Area14013;
    func_801F3950(((u8 *)tailWork)[8], (s16)tailWork[0x1F]);
}
