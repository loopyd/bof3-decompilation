#include "bof3/bof3.h"

/* Record pool of this overlay at 0x801F00A0: 64 records of stride 0x78 whose
 * byte 0 is the flags byte that reserveFirstFreeRecordBmagicMagic10503_801EFC8C
 * tests for bit 0 and sets on reservation. Byte 0 plus the two bytes that follow
 * it are cleared here for every record. The map row naming the pool base is this
 * target's magic10503RecordFlags, and the two following bytes are bound by the
 * address-anchored names this target's generated undefined_syms_auto.txt already
 * carries for them, so no map row is added. */
extern u8 magic10503RecordFlags[]; /* @source 0x801F00A0 @kind data */
extern u8 D_801F00A1[]; /* @source 0x801F00A1 @kind data */
extern u8 D_801F00A2[]; /* @source 0x801F00A2 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The original re-materialises the cell address at each
 * access group (lui $vN,0x1F80 / lw $vN,68($vN)) - once for the word-0x34 store
 * group, once before the pointer publication of the claim loop and once for the
 * byte-0xB count that loop advances - because a store through the loaded
 * pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is added. This body copies only byte 0x08 of that record,
 * which is the byte the exact sibling
 * copyWorkTripletSeedOffsetPairAndAdvanceWorkByte2BmagicMagic10503_801EF06C
 * reads for its own seeded fields. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared fixed-point pair in main RAM, outside this overlay's image, and the
 * point this body projects the work pose onto. The exact siblings
 * seedWorkWordPairAndFlagsThenAdvanceWorkByte2BmagicMagic13103_801F04C0 and
 * seedWorkChannelPairDistanceRandomByteBCountdownAndAdvanceWorkByte1BmagicMagic08000_801EEC5C
 * copy the same two words into the same work words 0x34 and 0x38; here they are
 * added to work words 0x0C and 0x10 instead, and the word differences back to
 * this pair feed the direction helper. Both addresses are bound by this target's
 * generated undefined_syms_auto.txt rows, so no target-map row is added. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Byte 1, byte 4 and word 0x74 of the 0x78-byte-stride action-slot record pool
 * based at 0x801EC330 in the shared main executable. The original reaches each
 * field through its own flat symbol - lui $at,%hi(SYM), addu $at,$at,index*0x78,
 * store %lo(SYM)($at) - with the scaled index shared by the three stores, so
 * every addressed field keeps its own address-anchored name instead of a shared
 * record base, exactly as the exact sibling
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndAdvanceHandlerIndexBmagicMagic08103_801EEEDC
 * declares bytes 1 and word 0x74 of the same pool. All three addresses are bound
 * by this target's generated undefined_syms_auto.txt rows. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-RAM byte outside this overlay's image, bound by this target's
 * generated undefined_syms_auto.txt row D_80145988 = 0x80145988. Other lifted
 * targets in this tree describe it as a stage serial, but nothing in this target
 * establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Shared main-executable helpers, all outside this overlay's image and bound by
 * this target's generated undefined_funcs_auto.txt rows. func_800AFD9C is called
 * with no argument set up, func_801DD350 takes the scratchpad work-object pointer
 * this body just seeded, and func_8017A138 is the direction helper the exact
 * sibling
 * decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionToward8014638CClearByte9AndAdvanceWorkByte2BmagicMagic17203_801EF590
 * declares with the same two signed-word arguments and stores into work word
 * 0x0C. func_801E590C is the action-slot allocator the exact sibling
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndAdvanceHandlerIndexBmagicMagic08103_801EEEDC
 * calls with two selectors and whose zero-extended low byte indexes the pool, and
 * func_8015D71C is the helper that body of the same family invokes with 0x100. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EED1C
 * @behavior Entry stage 0 of the MAGIC105 slot 3 overlay: word 0 of the
 *           two-entry handler table func_801EEC14 builds in its own frame and
 *           calls through work-object byte 0x1 scaled by four, so no jal in the
 *           payload names it. It clears bytes 0x00, 0x01 and 0x02 of all 64
 *           records of the 0x78-byte-stride pool at 0x801F00A0 with one sb per
 *           byte, computing the stride from the masked byte counter and testing
 *           that counter against 0x40 at the loop bottom. It then calls the
 *           shared main-executable helper func_800AFD9C with no arguments,
 *           copies byte 0x08 of the shared battle record reached through the
 *           pointer cell 0x801EB4E0 into the work object's byte 0x08, and seeds
 *           work word 0x0C with 0xFFF80000 and work word 0x10 with zero before
 *           handing the same work pointer to func_801DD350. It publishes the
 *           work pose by adding the shared pair at 0x80149308 and 0x8014930C to
 *           work words 0x0C and 0x10 and storing the sums into work words 0x34
 *           and 0x38, then takes the word differences back to that pair and
 *           publishes the func_8017A138 direction of them in work word 0x0C,
 *           clearing work byte 0x0B after the direction store. It then claims
 *           seven action-slot records through the shared allocator
 *           func_801E590C(1, 0x33): each zero-extended low byte of the answer is
 *           scaled by 0x78 and receives a cleared byte 1, the loop counter in
 *           byte 4 and the work-object pointer in word 0x74, and the body
 *           advances work byte 0x0B once per claim. It copies the sixteen
 *           unsigned halfwords of the table at 0x80036C00 into the table at
 *           0x8003AC00 with bit 15 set on every entry, restores entry 0 of the
 *           destination to zero, stores 1 into the shared main-RAM byte
 *           0x80145988, invokes the main-executable helper func_8015D71C with
 *           0x100, and increments work byte 0x01, the handler index this
 *           dispatcher tests, so the next frame runs the other entry. Takes no
 *           arguments and returns nothing; the 0x18-byte frame keeps ra at 0x14
 *           and the byte counter at 0x10 across the five calls. The roles of
 *           pool bytes 0x01 and 0x02, of work byte 0x08, of the two halfword
 *           tables and of byte 0x80145988 are not established here, so they stay
 *           address-anchored.
 * @status partial
 * @match 89.76
 * @residual Measured live at 114 of 127 instructions in 508 bytes, the
 *           original size. Every instruction of the original is present, in
 *           order, with the same memory accesses and calls; the thirteen that
 *           still differ are register choices and one slot. The record-clear
 *           loop emits its latch addiu after the stride chain where the
 *           original emits it before, and the work-pointer loads of the
 *           byte-0x08 group, of the post-direction store group and of the
 *           claim loop sit in $a0 where the original holds $v1, $v1 and $v0,
 *           while the claim loop's count load sits in $v1 where the original
 *           holds $a0, which also leaves that load one nop of padding. Source
 *           shapes measured at this same first difference: for, while,
 *           do-while, goto and for-with-break loops, u8, u16 and u32 counters
 *           with explicit byte casts, five declaration orders, one shared
 *           work pointer against one per group, and four claim-loop store
 *           orders. Statement order and operand forms follow the exact
 *           siblings named above.
 */
void clearRecordPoolBytes0To2SeedWorkPoseAndDirectionClaimSevenSlotsCopy16HalfwordsSettingBit15SetByte80145988ToOneInvokeHelperAt100ThenAdvanceWorkByte1BmagicMagic10503_801EED1C(
    void) {
    u8 index;
    u32 offset;
    u8 *record;
    u8 *work;
    u8 *next;
    u8 *pos;
    u8 *tail;
    u8 *slot;
    u8 *count;
    s32 direction;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        magic10503RecordFlags[offset] = 0;
        D_801F00A1[offset] = 0;
        D_801F00A2[offset] = 0;
    }
    func_800AFD9C();
    record = D_801EB4E0;
    work = g_battle_work;
    work[8] = record[8];
    next = g_battle_work;
    *(u32 *)(next + 0xC) = 0xFFF80000;
    *(u32 *)(next + 0x10) = 0;
    func_801DD350(next);
    pos = g_battle_work;
    *(u32 *)(pos + 0x34) = D_80149308[0] + *(u32 *)(pos + 0xC);
    *(u32 *)(pos + 0x38) = D_8014930C[0] + *(u32 *)(pos + 0x10);
    direction = func_8017A138(D_80149308[0] - *(s32 *)(pos + 0x34),
                              D_8014930C[0] - *(s32 *)(pos + 0x38));
    tail = g_battle_work;
    *(s32 *)(tail + 0xC) = direction;
    tail[0xB] = 0;
    for (index = 0; index < 7; index++) {
        offset = (u8)func_801E590C(1, 0x33) * 0x78;
        slot = g_battle_work;
        D_801EC331[offset] = 0;
        D_801EC334[offset] = index;
        count = g_battle_work;
        *(u8 **)(D_801EC3A4 + offset) = slot;
        count[0xB] = count[0xB] + 1;
    }
    for (index = 0; index < 0x10; index++) {
        ((u16 *)0x8003AC00)[index] = ((u16 *)0x80036C00)[index] | 0x8000;
    }
    *(u16 *)0x8003AC00 = 0;
    D_80145988 = 1;
    func_8015D71C(0x100);
    g_battle_work[1]++;
}
