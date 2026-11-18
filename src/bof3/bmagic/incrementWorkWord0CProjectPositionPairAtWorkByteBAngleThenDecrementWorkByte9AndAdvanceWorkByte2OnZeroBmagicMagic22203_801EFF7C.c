#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. This body reads the
 * cell four times: once for the angle and speed it publishes, once for the
 * first projection, once for the second projection and the countdown, and once
 * more for the byte-2 advance, because the called sample routines and the
 * handlers they reach may republish it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword 4, the angle cell this body publishes before it samples
 * 0x801782FC, so the paired routine 0x801783C8 reads the same turn back through
 * a plain halfword load for its axis. This target's generated undefined-syms
 * map already binds the address. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Paired sine/cosine table samplers in the shared main executable, declared at
 * their own addresses as target-local prototypes: they lie outside this
 * payload, so their names stay raw and no map row is owed for them. */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */

/* @source 0x801EFF7C
 * @behavior Per-frame work-handler step, entry 1 of the eight-entry handler
 *           table based at 0x801F0D8C whose word at 0x801F0D90 holds this
 *           function's address, so the byte-1 dispatcher
 *           dispatchWorkByte2HandlerThenDrawTransformedAndPopWhenWorkBytes0And2Set
 *           BmagicMagic22203_801EFDEC selects it by work byte 2 while that
 *           object's byte 1 is 2 and no jal in this payload targets this
 *           address. Reads the scratchpad work object through the pointer cell
 *           at 0x1F800044 before it creates its frame, takes work byte 0xB as
 *           the sample angle shifted left nine into the halfword at
 *           0x1F800004, and adds one to work word 0x0C. It samples 0x801782FC
 *           at that angle truncated to sixteen bits, re-reads the cell and
 *           stores the shared record's word 0x34 plus that sample times work
 *           word 0x0C into work word 0x34. It then samples 0x801783C8 at the
 *           signed scratchpad halfword
 *           it published, and stores the shared record's word 0x38 plus that
 *           sample times work word 0x0C into work word 0x38, the store filling
 *           the branch delay slot. Finally it pre-decrements work byte 9 at byte
 *           width with the stored value truncated to eight bits, and when that
 *           countdown reaches zero it re-reads the cell and adds one to work
 *           byte 2, which moves the dispatcher to the next byte-2 step. The
 *           0x18-byte frame keeps $ra at 0x14 and $s0 at 0x10, $s0 holding the
 *           scratchpad angle cell across both calls; it takes no arguments,
 *           returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkWord0CProjectPositionPairAtWorkByteBAngleThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic22203_801EFF7C(void) {
    u8 *work;
    u8 *work2;
    u8 *work3;
    s16 *angle;
    s32 direction;
    s32 sample;
    u8 counter;

    work = g_battle_work;
    angle = &D_1F800004;
    direction = work[0xB];
    *(s32 *)(work + 0x0C) += 1;
    *angle = direction << 9;
    sample = func_801782FC((s16)(direction << 9));
    work2 = g_battle_work;
    *(s32 *)(work2 + 0x34) = *(s32 *)(D_801EB4E0 + 0x34) + sample * *(s32 *)(work2 + 0x0C);
    sample = func_801783C8(*angle);
    work3 = g_battle_work;
    *(s32 *)(work3 + 0x38) = *(s32 *)(D_801EB4E0 + 0x38) + sample * *(s32 *)(work3 + 0x0C);
    counter = work3[9] - 1;
    work3[9] = counter;
    if (counter == 0) {
        g_battle_work[2]++;
    }
}
