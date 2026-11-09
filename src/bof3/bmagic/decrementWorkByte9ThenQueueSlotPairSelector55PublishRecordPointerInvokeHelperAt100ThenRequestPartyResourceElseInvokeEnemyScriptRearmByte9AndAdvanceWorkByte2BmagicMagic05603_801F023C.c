#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers, bound at 0x1F800044 by this target's map. It keeps the volatile
 * pointer spelling the exact bmagic siblings
 * runSharedUpdateThenDecrementWorkByte9AndRaiseFlag20SetByte5CToOneClearWork
 * ColorTripletAndAdvanceWorkByte2BmagicMagic05603_801F042C and
 * runSharedUpdateThenDecrementWorkColorTripletBy16AndCopyRecordTripletRequest
 * SlotResourceElseInvokeEnemyScriptAndAdvanceWorkByte2WhenChannelIs80Bmagic
 * Magic05603_801F04EC use: every statement that goes through it re-reads the
 * cell, because a store through the pointer it supplied may alias the cell
 * itself. This body also publishes the selected party work record into the cell
 * for the duration of one resource request and writes the saved work object
 * back afterwards. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Acting battler byte of the main executable, outside this overlay's image.
 * Party slots are 0, 1 and 2 and the enemy panel slots are 3 and above, so the
 * two unsigned compares against 3 below select the party work base this span
 * publishes, and the enemy arm reads the same byte a third time as its own
 * argument. The array spelling is the measured one: the original keeps the
 * address in one register across the helper call and loads through it twice,
 * which a byte scalar does not produce, and the shared enemy arm reloads the
 * byte because the first path to it never loaded it. Splat binds the address
 * through this target's generated undefined_syms_auto.txt, so no target-map row
 * is needed. */
extern u8 D_80146374[]; /* @source 0x80146374 @kind data */

/* Base of the three 0x140-byte-stride party work records the acting battler byte
 * selects; the same target's 0x801F04EC indexes the same base the same way. The
 * original reuses the loaded base across the helper call, so this body reaches
 * it through one kept address. Splat binds the address through this target's
 * generated undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */

/* Current work-record pointer cell of the main executable. The world and plchar
 * lifts read the same cell for the record byte 0x12B. This span saves the cell,
 * publishes the selected party record into it for the duration of
 * func_801DD858, and writes the saved pointer back afterwards. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is needed. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* Shared battle record pointer cell of the main executable: it holds the record
 * this overlay's handlers queue. The same target's magic069 queue lifts store
 * that pointer into the queued slot and advance its byte 0xB, the count of
 * entries queued from the record. The original re-reads the cell for the byte
 * 0xB advance instead of reusing the pointer it just stored, so the
 * declaration keeps the plain pointer spelling. Splat binds the address through
 * this target's generated undefined_syms_auto.txt. */
extern u8* D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Mode byte and work-pointer word of the queued action-slot store at
 * 0x801EC330, whose 0x78-byte stride these two accesses scale by. The original
 * reaches both with `lui $at,%hi(SYM)` + `addu at,at,offset` + `sb|sw
 * %lo(SYM)(at)` rather than a shared record base, so each addressed byte keeps
 * its own flat symbol; the two are byte +0x01 and word +0x74 of the same entry.
 * This target's generated undefined_syms_auto.txt carries both addresses. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared queued-slot allocator of the main executable: it takes a mode word and
 * a selector byte and returns the index of the slot it claimed. The bmagic and
 * battle lifts declare the same signature; this span passes mode 1 with selector
 * 0x55 and narrows the answer to its byte. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Shared main-executable helper selected by the 0x100 id; the bmagic lifts call
 * the same entry with 0x100 and 0x101 and keep none of its answer. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* Shared resource-request helper of the main executable. The same target's
 * 0x801F04EC passes a party record byte plus 0x1C through it; this span passes
 * one plus 0x10, which is why the declaration narrows its argument to byte
 * width. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt. */
extern void func_8014D6B8(u8 resource); /* @source 0x8014D6B8 */

/* Shared battle helper of the main executable: it publishes the 0x118-byte enemy
 * work record at 0x801EB2E8 indexed by its first argument and runs the enemy
 * script byte helper for the second. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt. */
extern void func_801E257C(u8 index, u8 scriptByte); /* @source 0x801E257C */

/* Main-executable helper this span runs once with argument 0 between publishing
 * the party record into the current-record cell and restoring that cell. No
 * other lifted source in the tree references this address, so its role is not
 * established and the name stays address-anchored. */
extern void func_801DD858(u32 arg0); /* @source 0x801DD858 */

/* Measured shape notes for this span. The two action-slot claims keep one local
 * pair each instead of reusing a shared pair: a shared pair made the allocator
 * give both arms the same two registers and moved ten words, while the per-arm
 * pairs reproduce the original's v0/a2 then a0/a1 load targets. The two
 * acting-battler guards were seeded as one short-circuit condition with the
 * record publish inline, because writing them as two statements gave the first
 * guard the second test as its branch target where the original branches to the
 * shared enemy arm. Both reads are kept because the intervening helper call ends
 * the first read's availability. */

/* @source 0x801F023C
 * @behavior Step 4 of the nine-word code pointer run based at 0x801EEC10, which
 *           this target's work-byte-2 dispatcher
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2Set
 *           BmagicMagic05603_801EFF28 copies into its frame and selects from, so
 *           no jal in this payload names this address. The body first decrements
 *           the scratchpad work object's byte 9 at byte width and runs the rest
 *           only when that countdown reaches zero. On expiry it claims two queued
 *           action slots through func_801E590C with mode 1 and selector 0x55,
 *           scaling each returned index by the 0x78-byte slot stride, setting the
 *           slot's mode byte 1 to 0 for the first claim and to 2 for the second,
 *           storing the shared battle record pointer from 0x801EB4E0 into the
 *           slot's +0x74 word, and advancing that record's byte 0xB each time. It
 *           then runs the main-executable helper selected by 0x100. It reads the
 *           acting battler byte at 0x80146374 and, while that byte is below 3,
 *           publishes the matching 0x140-byte party work record into the current
 *           record cell at 0x80146250 as the argument of func_801DD858, restoring
 *           the saved cell afterwards. It re-reads the acting battler byte and,
 *           while it is still below 3, publishes the matching party record as the
 *           scratchpad work object, asks func_8014D6B8 for that record's byte 8
 *           plus 0x10, restores the saved work object and asks func_8014D6B8
 *           again with that object's byte 8 plus 0x10. Once the acting battler
 *           byte is 3 or above, which is the enemy panel, it instead calls the
 *           shared battle helper func_801E257C with the acting battler byte as
 *           enemy work index and 4 as the script byte. Both guards branch to
 *           that one enemy arm, so it runs when either read of the acting
 *           battler byte reaches 3, and it reloads the byte for its argument.
 *           Both surviving paths then re-read the work object, set its byte 9 to
 *           0x1E and increment its byte 2 at byte width, rearming the step
 *           countdown and advancing the work-byte-2 dispatcher to step 5. The
 *           0x20-byte frame keeps the return address at 0x1C and the saved work
 *           object, acting-battler address and party base at 0x10, 0x14 and
 *           0x18. Takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenQueueSlotPairSelector55PublishRecordPointerInvokeHelperAt100ThenRequestPartyResourceElseInvokeEnemyScriptRearmByte9AndAdvanceWorkByte2BmagicMagic05603_801F023C(void) {
    u8* work;
    u8* record; /* cell read published into the first claim's slot word. */
    u8* saved;
    u8* party;
    u8* queued; /* second cell read of the first claim, its byte 0xB counter. */
    u8* record2; /* the second claim repeats both reads with its own locals. */
    u8* queued2;
    u32 offset;
    u8 index;

    if (--g_battle_work[9] == 0) {
        offset = (u8)func_801E590C(1u, 0x55u) * 0x78;
        record = D_801EB4E0;
        D_801EC331[offset] = 0;
        queued = D_801EB4E0;
        *(u32*)&D_801EC3A4[offset] = (u32)record;
        queued[0xB]++;

        offset = (u8)func_801E590C(1u, 0x55u) * 0x78;
        record2 = D_801EB4E0;
        D_801EC331[offset] = 2;
        queued2 = D_801EB4E0;
        *(u32*)&D_801EC3A4[offset] = (u32)record2;
        queued2[0xB]++;

        func_8015D71C(0x100);

        index = D_80146374[0];
        if (index < 3u &&
            (saved = D_80146250,
             D_80146250 = &D_80145E90[index * 0x140],
             func_801DD858(0),
             D_80146250 = saved,
             (index = D_80146374[0]) < 3u)) {
            work = g_battle_work;
            party = &D_80145E90[index * 0x140];
            g_battle_work = party;
            func_8014D6B8(party[8] + 0x10);
            g_battle_work = work;
            func_8014D6B8(work[8] + 0x10);
        } else {
            func_801E257C(D_80146374[0], 4);
        }

        g_battle_work[9] = 0x1E;
        g_battle_work[2]++;
    }
}
