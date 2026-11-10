#include "bof3/bof3.h"

/* Record pool of this overlay at 0x801F0424: 80 records of stride 0x78 whose
 * byte 0 holds the flag bit that the frame dispatcher func_801EEC38 tests
 * before it runs a record, and whose word 0x74 that dispatcher publishes
 * through the shared battle-record cell 0x801EB4E0. Byte 0 plus the two bytes
 * that follow it are cleared here for every record. The map carries no row at
 * this address and the pool base is generated from the name the payload's own
 * asm used, so the three cleared bytes stay address-anchored. */
extern u8 D_801F0424[]; /* @source 0x801F0424 @kind data */
extern u8 D_801F0425[]; /* @source 0x801F0425 @kind data */
extern u8 D_801F0426[]; /* @source 0x801F0426 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body reads the cell once for the word 0x3C update and
 * again before each of the three byte accesses that follow the helper call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-RAM byte outside this overlay's image, generated from the name
 * the payload's own asm used. Other lifted targets in this tree describe it as
 * a stage serial, but nothing in this target establishes that role, so the name
 * keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Shared main-executable helper at 0x800AFD9C, outside this overlay's image.
 * The plchar and bmagic lifts
 * invokeHelperWhenFlag4SetElseInvokeMainExecutableHelperBmagicMagic08103_801EF2F0
 * and seedWorkByte8FromRecordThenQueueSixteenSlotsAndAdvanceWorkByte1BmagicMagic10103_801EECCC
 * call it with no arguments and keep no result, so its role stays
 * address-anchored here and no target-map row is added. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */

/* Shared main-executable helper at 0x8015D71C, outside this overlay's image.
 * When this seed was written the same target's sibling lift
 * incrementWorkByteARecordCountersThenInvokeHelperOnCountdownZeroAndAdvanceWorkByte2BmagicMagic16903_801EF4FC
 * had already gated exact and declared the helper as a void function taking one
 * u16 selector, so the call site here passes its literal through that same
 * declaration. */
extern void func_8015D71C(u16 selector); /* @source 0x8015D71C */

/* @source 0x801EED8C
 * @behavior Entry stage 0 of this overlay: word 0 of the thirteen-word code
 *           pointer run at 0x801EEC04; the frame dispatcher func_801EEC38
 *           copies that run's first four words onto its own stack and calls
 *           through the work-object byte 0x1 scaled by four, so no jal in this
 *           payload names the body. It
 *           clears bytes 0x00, 0x01 and 0x02 of all 80 records of the
 *           0x78-byte-stride pool at 0x801F0424 with one sb per byte, computing
 *           the stride from the masked index and testing the incremented index
 *           against 0x50 at the loop bottom. It then calls the shared
 *           main-executable helper func_800AFD9C with no arguments and adds
 *           0xC00000 to word 0x3C of the work object read from the scratchpad
 *           pointer cell 0x1F800044. Next it copies the 256 unsigned halfwords
 *           of the table at 0x80036C00 into the table at 0x8003AC00 with bit 15
 *           set on every entry, stores 1 into the shared main-RAM byte
 *           0x80145988, restores entry 0 of the destination to the source value
 *           without that bit, and invokes the main-executable helper
 *           func_8015D71C with 0x100. Last it clears work bytes 9 and 0xB,
 *           reloading the scratchpad cell before each store, and increments work
 *           byte 0x1, the handler index this overlay dispatches on, so the next
 *           frame runs the following entry. Takes no arguments and returns
 *           nothing; the 0x18-byte frame keeps ra at 0x10 across the two calls.
 *           The roles of pool bytes 0x01 and 0x02, of the two halfword tables
 *           and of byte 0x80145988 are not established here, and the two
 *           helpers are named by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordPoolBytes0To2InvokeSharedHelperAddC00000ToWorkWord3CCopy256HalfwordsSettingBit15SetByte80145988ToOneCopyFirstHalfwordInvokeHelperAt100ClearWorkBytes9AndBAdvanceWorkByte1BmagicMagic16903_801EED8C(
    void) {
    u16 index;
    u32 offset;
    u8 *work;
    u8 *clear9;
    u8 *clearB;
    u8 *step;

    for (index = 0; index < 0x50; index++) {
        offset = index * 0x78;
        D_801F0424[offset] = 0;
        D_801F0425[offset] = 0;
        D_801F0426[offset] = 0;
    }
    func_800AFD9C();
    work = g_battle_work;
    *(u32 *)(work + 0x3C) = *(u32 *)(work + 0x3C) + 0xC00000;
    for (index = 0; index < 0x100; index++) {
        PSX_PTR(u16, 0x8003AC00u)[index] =
            PSX_PTR(u16, 0x80036C00u)[index] | 0x8000;
    }
    D_80145988 = 1;
    PSX_REF(u16, 0x8003AC00u) = PSX_REF(u16, 0x80036C00u);
    func_8015D71C(0x100);
    clear9 = g_battle_work;
    clear9[9] = 0;
    clearB = g_battle_work;
    clearB[0xB] = 0;
    step = g_battle_work;
    step[1]++;
}
