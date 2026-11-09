#include "bof3/bof3.h"

u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
void func_8015D71C(u32 arg0); /* @source 0x8015D71C */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's separate reads
 * each re-materialise the base. The arm reloads the cell three times: once for
 * the queued pointer, once for the work object whose byte 0xB it increments,
 * and twice more for the rearm and the state-byte advance, each separated from
 * the previous read by a store through the loaded pointer or by a call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* State byte and work-pointer word of the queued action-slot store at
 * 0x801EC330, whose 0x78-byte stride these two accesses scale by. The original
 * reaches both with `lui $at,%hi(SYM)` + `addu at,at,index*0x78` + `sb|sw
 * %lo(SYM)(at)` rather than a shared record base, so each addressed byte keeps
 * its own flat symbol; the two are byte +0x01 and word +0x74 of the same entry.
 * This target's generated undefined_syms_auto.txt carries both addresses because
 * the asm handlers func_801EFAAC and func_801EEF24 still reference them. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* @source 0x801EFC68
 * @behavior Countdown step of the overlay work object, and the third entry of
 *           the six-entry work-byte-2 handler table at 0x801EEC38 that
 *           dispatchWorkByte2HandlerBmagicMagic15103_801EFA34 copies. The
 *           scratchpad work-object pointer cell at 0x1F800044 is read once,
 *           before this function's 0x18-byte frame is created, and the
 *           work-object byte at offset 0x9 is decremented through that pointer
 *           with a byte-width read-modify-write (lbu / addiu -1 / sb); the
 *           truncated byte it just stored is tested (andi 0xFF of the stored
 *           register, no second load of the cell) and the function returns
 *           while it is still nonzero. On expiry it queues a battle action-slot
 *           entry through func_801E590C(1, 0x44) - mode byte 1 in entry byte
 *           +0x06 and 0x44 in entry byte +0x05 - and scales the returned index
 *           by the 0x78-byte entry stride. Entry byte +0x01 is cleared to zero
 *           and the work-object pointer is published as the entry's pointer
 *           word at +0x74; both slot stores use the same scaled offset and the
 *           cell is read again between them. The step then increments the
 *           work-object byte at offset 0xB through that second read, invokes
 *           func_8015D71C with the constant 0x103, re-reads the cell and stores
 *           0x14 into the work-object byte at offset 0x9, re-reads the cell once
 *           more and increments the work-object state byte at offset 0x2,
 *           advancing the overlay work byte 2 handler sequence. The clear, the
 *           published pointer and the rearm all differ from the sibling entry
 *           0x801EFBAC (byte +0x01 set to 3, byte 0x9 set to 4, no helper call),
 *           which is why this body is written from this span's own bytes. The
 *           arm's cell reads stay in separate locals because the original
 *           reloads the cell for each; a single local folded a second read in
 *           the sibling. Frame 0x18 with $ra only, because of the two calls; it
 *           takes no arguments, returns nothing and reads or writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenQueueSlotClearByte1PublishWorkPointerIncrementWorkByteBInvokeHelperAt103RearmByte9To20AndAdvanceWorkByte2WhenZeroBmagicMagic15103_801EFC68(void) {
    u8 *work;
    u8 *queued_work;
    u8 *arm_work;
    u32 slot;
    u32 offset;
    u8 countdown;

    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
        slot = (u8)func_801E590C(1u, 0x44u);
        offset = slot * 0x78;
        queued_work = g_battle_work;
        D_801EC331[offset] = 0;
        arm_work = g_battle_work;
        *(u32 *)&D_801EC3A4[offset] = (u32)queued_work;
        arm_work[0xB]++;
        func_8015D71C(0x103);
        g_battle_work[9] = 0x14;
        g_battle_work[2]++;
    }
}
