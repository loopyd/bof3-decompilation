#include "bof3/bof3.h"

/* The overlay's fixed-point table: twelve 16.16 words from 0x801D9688 followed
 * by the in-image code pointer run that fills the head of the reviewed
 * `textbin` row T_801D9688. This body indexes it with the work byte 0x801DA033,
 * which no other code of this payload reads, so the reachable index range stays
 * unbounded here.
 */
extern u32 T_801D9688[]; /* @source 0x801D9688 @kind table */

/* The panel step byte of the work block 0x801DA028..0x801DA12D: this body
 * multiplies it by three to step the record's three channels; the sibling step
 * byte 0x801DA02E drives the first record's channels the same way.
 */
extern u8 D_801DA02F; /* @source 0x801DA02F @kind data */

/* The table index byte of that block, read by this body alone. */
extern u8 D_801DA033; /* @source 0x801DA033 @kind data */

/* The three channel bytes of the second work record at 0x801DA0C0: this body
 * lowers the first and third toward zero and raises the second toward 0xFF by
 * three times the step byte, then writes the saved bytes back, so the stepped
 * values are visible to the drawing call alone. The first record at 0x801DA048
 * carries the same fields at 0x801DA0A5 through 0x801DA0A7.
 */
extern u8 D_801DA11D; /* @source 0x801DA11D @kind data */
extern u8 D_801DA11E; /* @source 0x801DA11E @kind data */
extern u8 D_801DA11F; /* @source 0x801DA11F @kind data */

/* The 16.16 cell of that record: the fixed-point table word this body selects
 * goes here, at the same offset as the first record's cell 0x801DA088. */
extern u32 D_801DA100; /* @source 0x801DA100 @kind data */

/* The second work record's base. The drawing routine takes it as its only
 * argument; the first record's base is 0x801DA048, 0x78 bytes earlier. */
extern u8 D_801DA0C0[]; /* @source 0x801DA0C0 @kind data */

/* The overlay's shared drawing routine, still an `asm` row of its Splat layout,
 * so only its call is bound here. Its bytes push a matrix, publish the record
 * word at +0x40 into the colour vector and draw the record it is handed.
 * @source 0x801D2308
 */
extern void func_801D2308(void *record);

/* @source 0x801D2ADC
 * @behavior Drawing step of the SHISU overlay for its second work record at
 *           0x801DA0C0: it stores the 16.16 word that the work byte 0x801DA033
 *           indexes in the fixed-point table based at T_801D9688 into the
 *           record cell 0x801DA100, saves the record's three channel bytes
 *           0x801DA11D through 0x801DA11F on its frame, steps the first and
 *           third down by three times the panel byte 0x801DA02F to a floor of
 *           zero and the second up by the same amount to a ceiling of 0xFF,
 *           calls the shared drawing routine func_801D2308 with the record at
 *           0x801DA0C0, and writes the three saved bytes back, so the stepping
 *           reaches the drawing call while the record keeps its stored values.
 *           This payload reaches it from the five jal sites at 0x801D2E08,
 *           0x801D2E68, 0x801D2F2C, 0x801D2F50 and 0x801D2F84, all in the
 *           second record's step handlers, and no word or jal names it from
 *           outside; its 0x20-byte frame only keeps $ra at 0x18 across the
 *           drawing call and the epilogue leaves that call's result in $v0.
 * @status partial
 * @match 98.68
 * @residual The second channel's byte store is the one instruction out of
 *           place: the post-reload scheduler lifts the third channel's
 *           pointer build (lui/addiu into a1) above that store, so the store
 *           lands two slots later than the original's, which keeps it at the
 *           branch merge before the build. Every other instruction, register
 *           and the 0x130-byte size match. The store has no dependent and
 *           both it and the build carry priority 1 in the sched2 dump, so the
 *           hoist is a hazard tie-break; a volatile store, a signed view
 *           without the local pointer, do/while loop scopes around the store
 *           or around the third channel, one shared scope for both channels,
 *           statement-order swaps and a top-level pointer assignment each
 *           measured lower (see the lane report).
 */
void rampShisuWorkRecordChannelsThenDrawShisu00_801D2ADC(void)
{
    u8 saved[3];

    D_801DA100 = T_801D9688[D_801DA033];
    saved[0] = D_801DA11D;
    saved[1] = D_801DA11E;
    saved[2] = D_801DA11F;
    if (D_801DA11D > D_801DA02F * 3)
        D_801DA11D = D_801DA11D - D_801DA02F * 3;
    else
        D_801DA11D = 0;
    {
        u8 *p = &D_801DA11E;

        *(s8 *)p = (0xFF - *p > D_801DA02F * 3) ? *p + D_801DA02F * 3 : -1;
    }
    {
        u8 *q = &D_801DA11F;

        if (*q > D_801DA02F * 3)
            *q = *q - D_801DA02F * 3;
        else
            *q = 0;
    }
    func_801D2308(D_801DA0C0);
    D_801DA11D = saved[0];
    D_801DA11E = saved[1];
    D_801DA11F = saved[2];
}
