#include "bof3/bof3.h"

u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's separate reads
 * each re-materialise the base. The plain cell is kept rather than the volatile
 * one several siblings use: every read here but the arm's second one is
 * separated from the previous read by a store through the loaded pointer, and
 * the arm's pair is separated by the slot state-byte store, so the reloads come
 * from the stores and not from a qualifier. Holding those two arm reads in
 * separate locals is what keeps them apart: one shared local folded the second
 * read into the first and measured 180 bytes. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* State byte and work-pointer word of the queued action-slot store at
 * 0x801EC330, whose 0x78-byte stride these two accesses scale by. The original
 * reaches both with `lui $at,%hi(SYM)` + `addu at,at,index*0x78` + `sb|sw
 * %lo(SYM)(at)` rather than a shared record base, so each addressed byte keeps
 * its own flat symbol; the two are byte +0x01 and word +0x74 of the same entry.
 * This target's generated undefined_syms_auto.txt carries both addresses because
 * the asm handlers func_801EFAAC, func_801EEF24, func_801EFC68 and func_801EFF94
 * still reference them. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* @source 0x801EFBAC
 * @behavior Work-step handler of the overlay work object. The scratchpad
 *           work-object pointer cell at 0x1F800044 is read once, before this
 *           function's 0x18-byte frame is created, and the work-object byte at
 *           offset 0x9 is decremented through that pointer with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb); the truncated byte it just
 *           stored is tested (andi 0xFF of the stored register, no second load
 *           of the cell) and the function returns while it is still nonzero.
 *           On expiry it queues a battle action-slot entry through
 *           func_801E590C(1, 0x44) - mode byte 1 in entry byte +0x06 and 0x44 in
 *           entry byte +0x05 - and scales the returned index by the 0x78-byte
 *           entry stride. Entry byte +0x01 becomes 3, and the work-object
 *           pointer is published as the entry's pointer word at +0x74; both
 *           slot stores use the same scaled offset and the cell is read again
 *           for the second one. The step then increments the work-object byte
 *           at offset 0xB through that second read, re-reads the cell and stores
 *           4 into the work-object byte at offset 0x9, re-reads the cell once
 *           more and increments the work-object state byte at offset 0x2,
 *           advancing the overlay work byte 2 handler sequence. The arm's two
 *           cell reads stay in separate locals because the original reloads the
 *           cell for each; a single local measured 180 bytes. Frame 0x18 with
 *           $ra only, because of the call; it takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenQueueSlotSetByte1To3PublishWorkPointerAndAdvanceWorkByte2WhenZeroBmagicMagic15103_801EFBAC(void) {
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
        D_801EC331[offset] = 3;
        arm_work = g_battle_work;
        *(u32 *)&D_801EC3A4[offset] = (u32)queued_work;
        arm_work[0xB]++;
        g_battle_work[9] = 4;
        g_battle_work[2]++;
    }
}
