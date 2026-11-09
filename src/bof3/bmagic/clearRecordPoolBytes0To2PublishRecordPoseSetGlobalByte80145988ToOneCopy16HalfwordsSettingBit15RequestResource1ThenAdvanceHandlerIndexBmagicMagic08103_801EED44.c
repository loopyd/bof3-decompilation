#include "base/types.h"

/* Record pool of this overlay at 0x801EFA8C: 48 records of stride 0x78 whose
 * byte 0 is the flags byte that reserveFirstFreeRecordBmagicMagic08103_801EFA20
 * tests for bit 0 and sets on reservation. Byte 0 plus the two bytes that follow
 * it are cleared here for every record. The map row names the pool base, and the
 * two following bytes are bound by the address-anchored names the generated
 * undefined_syms_auto.txt carries for them. */
extern u8 magic08103RecordFlags[]; /* @source 0x801EFA8C @kind data */
extern u8 D_801EFA8D[]; /* @source 0x801EFA8D @kind data */
extern u8 D_801EFA8E[]; /* @source 0x801EFA8E @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body reads the cell once for the record pose, reads it
 * again before publishing the saved pointer and restores that second value once
 * after the helper call. The plain declaration is the one this target's exact
 * siblings use:
 *   publishSavedWorkPointerAdvancePreviousHandlerIndexWhenHelperReadyBmagicMagic08103_801EEE74
 *   copyRecordWordPairAdd2000ToWord34Add100ToHalfword3EThenSetDirectionTowardSlotRecordFromByteAStoreWord14ClearByte9AndAdvanceWorkByte2BmagicMagic08103_801EF39C */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. Words 0x34 and
 * 0x38 and halfword 0x3E of that record are the pose the sibling
 * copyRecordWordPairAdd2000ToWord34Add100ToHalfword3EThenSetDirectionTowardSlotRecordFromByteAStoreWord14ClearByte9AndAdvanceWorkByte2BmagicMagic08103_801EF39C
 * publishes the same way, with its biases added. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared battle record pointer in main RAM whose word 0x74 holds a saved
 * work-object pointer: the exact sibling
 * reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThenAdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C
 * stores the published work-object pointer into exactly that word, and this body
 * takes it back out. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Shared main-RAM byte outside this overlay's image, bound by this target's
 * generated undefined_syms_auto.txt row D_80145988 = 0x80145988. Other lifted
 * targets in this tree describe it as a stage serial, but nothing in this
 * target establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Shared main-executable helper at 0x8014D6B8, outside this overlay's image.
 * The plchar lifts requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelper
 * ReadyPlcharPlp03400_801CF84C and stepWorkByteBAndClearWorkByte2BWithRecord
 * State12BWhenStepReportsDonePlcharPlp67800_801CFC0C call it as the resource
 * request helper and keep no result, so its role stays address-anchored here.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added. */
extern void func_8014D6B8(u32 flag);

/* @source 0x801EED44
 * @behavior Entry stage 0 of this overlay: word 0 of the four-word pointer run
 *           at 0x801EEC04, which the frame dispatcher func_801EEC30 copies onto
 *           its own stack and calls through the work-object byte 0x1 scaled by
 *           four, so the body runs as that dispatcher stage 0 and no jal in the
 *           payload names it. It clears bytes 0x00, 0x01 and 0x02 of all 48
 *           records of the 0x78-byte-stride pool at 0x801EFA8C with one sb per
 *           byte, computing the stride from the masked byte counter and testing
 *           that counter against 0x30 at the loop bottom. It then takes the
 *           shared battle record through the pointer cell 0x801EB4E0 and the
 *           work object through the scratchpad pointer cell 0x1F800044, and
 *           publishes the record pose in the work object: work word 0x34
 *           receives the record word 0x34, work word 0x38 the record word 0x38
 *           and work halfword 0x3E the record unsigned halfword 0x3E, which the
 *           payload reads into a register before the serial store and writes
 *           after it. It
 *           publishes 1 into the shared main-RAM byte 0x80145988, then copies
 *           the sixteen unsigned halfwords of the table at 0x80036C00 into the
 *           table at 0x8003AC00 with bit 15 set on every entry, and restores
 *           entry 0 of the destination to the source value without that bit.
 *           Last it takes the shared battle record through the pointer cell
 *           0x801EC2E0, publishes that record word 0x74, the saved work-object
 *           pointer, through the scratchpad cell while keeping the cell previous
 *           value in a callee-saved register, requests resource 1 from the
 *           shared helper func_8014D6B8, restores the cell, and increments byte
 *           0x1 of the work object it published earlier, the handler index this
 *           overlay dispatches on, so the next frame runs the following entry.
 *           Takes no arguments and returns nothing; the 0x18-byte frame keeps
 *           ra at 0x14 and the saved cell value at 0x10 across the one call. The
 *           two table addresses, the roles of pool bytes 0x01 and 0x02 and the
 *           role of byte 0x80145988 are not established here, so they stay
 *           address-anchored, and the name reports the requested resource by its
 *           literal rather than by a game role.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordPoolBytes0To2PublishRecordPoseSetGlobalByte80145988ToOneCopy16HalfwordsSettingBit15RequestResource1ThenAdvanceHandlerIndexBmagicMagic08103_801EED44(void) {
    u8 index;
    u32 offset;
    u8 *record;
    u8 *work;
    u8 *saved;
    u8 *next;
    u16 pose;

    for (index = 0; index < 0x30; index++) {
        offset = index * 0x78;
        magic08103RecordFlags[offset] = 0;
        D_801EFA8D[offset] = 0;
        D_801EFA8E[offset] = 0;
    }
    record = D_801EB4E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
    pose = *(u16 *)(record + 0x3E);
    D_80145988 = 1;
    *(u16 *)(work + 0x3E) = pose;
    for (index = 0; index < 0x10; index++) {
        ((u16 *)0x8003AC00)[index] = ((u16 *)0x80036C00)[index] | 0x8000;
    }
    next = D_801EC2E0;
    saved = g_battle_work;
    *(u16 *)0x8003AC00 = *(u16 *)0x80036C00;
    g_battle_work = *(u8 **)(next + 0x74);
    func_8014D6B8(1);
    g_battle_work = saved;
    saved[1] = saved[1] + 1;
}
