#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work like the exact siblings of this target declare it. The body
 * takes three separate reads of the cell: one before the copies, which stays in
 * a register across all three stores, and two after the heading call, because a
 * store through the loaded pointer may alias the cell and the original
 * re-materialises the address with its own lui / lw pair at each of them. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image. This
 * target's generated undefined_syms_auto.txt binds the address and the sibling
 * reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThen
 * AdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C spawns
 * records through the same cell. This body reads the record word pair 0x34 and
 * 0x38, its unsigned halfword 0x3E and its byte 0xA; the roles beyond what the
 * body does with them are not proven, so the address stays address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Word fields 0x34 and 0x38 of the 0x78-byte-stride queued action-slot store
 * record based at 0x801EC330 in main RAM. The original reaches each field
 * through its own flat symbol - lui $at,%hi(SYM), addu $at,$at,index*0x78, load
 * %lo(SYM)($at) - with the scaled index in a single register shared by both
 * loads, so every addressed field keeps its own address-anchored name instead
 * of a shared record base, exactly as the sibling lift
 * copyQueuedSlotPositionPairAndDistanceThenAdvanceWorkByte2WhenSlotByte2IsTwo
 * BmagicMagic04500_801EF608 declares the same two addresses. This target's
 * generated undefined_syms_auto.txt carries both addresses while the selected
 * function is still an asm row. */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */

/* Two-argument heading helper of the main executable at 0x8017A138. The bmagic
 * siblings of other targets declare it as a signed-word pair and store its
 * answer as a heading, which is what this body does with work word 0x14.
 * Bound by this target's generated undefined_funcs_auto.txt row. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EF39C
 * @behavior Work byte 2 handler of this overlay, word 8 of the pointer run at
 *           0x801EEC04 that the overlay header publishes, so it is reached
 *           through the table and has no jal caller. It reads the shared battle
 *           record through the pointer cell at 0x801EB4E0 and the scratchpad
 *           work object through the cell at 0x1F800044, then publishes the
 *           record pose in the work object: work word 0x34 receives the record
 *           word 0x34 plus 0x2000, work word 0x38 receives the record word
 *           0x38, and work halfword 0x3E receives the record unsigned halfword
 *           0x3E plus 0x100. Both biases fit an addiu, so no lui is
 *           materialised. The three stores go through one cell read held in a
 *           register across them, and the record byte 0xA then selects the
 *           0x78-byte-stride queued action-slot store record: the byte is
 *           scaled once by the constant multiply the original expands as
 *           sll 4, subu, sll 3, and the two flat fields 0x801EC364 and
 *           0x801EC368 of the selected record are read through one shared
 *           offset register.
 *           func_8017A138 then takes the word differences between those two
 *           fields and the work words 0x34 and 0x38 re-read through the work
 *           pointer after the stores, and its answer is stored in work word
 *           0x14. Last the body clears work byte 9 through a fresh cell read,
 *           takes a second cell read that the heading store does not use, and
 *           advances work byte 2 through that second read, so the next frame
 *           runs the following entry of the pointer run. The two reads are
 *           visible in the payload as two lui 0x1F80 plus lw 0x44 pairs with
 *           the heading store between them, and that store fills the load delay
 *           slot of the second pair. The 0x18-byte frame keeps $ra at 0x10
 *           across the single call and saves no other register; the body takes
 *           no arguments, reads or writes no other state, and leaves the
 *           incremented byte 2 value in the return register from the byte
 *           increment alone. The roles of work bytes 9 and 2 and of word 0x14
 *           are not established beyond what this body does to them, so they
 *           stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordWordPairAdd2000ToWord34Add100ToHalfword3EThenSetDirectionTowardSlotRecordFromByteAStoreWord14ClearByte9AndAdvanceWorkByte2BmagicMagic08103_801EF39C(void) {
    u8 *record;
    u8 *work;
    u8 *tail;
    u8 *next;
    u8 index;
    s32 offset;
    s32 direction;

    record = D_801EB4E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) + 0x2000;
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
    *(u16 *)(work + 0x3E) = *(u16 *)(record + 0x3E) + 0x100;
    index = record[0xA];
    offset = index * 0x78;
    direction = func_8017A138(*(s32 *)(D_801EC364 + offset) - *(s32 *)(work + 0x34),
                              *(s32 *)(D_801EC368 + offset) - *(s32 *)(work + 0x38));
    tail = g_battle_work;
    tail[9] = 0;
    next = g_battle_work;
    *(s32 *)(tail + 0x14) = direction;
    next[2]++;
}
