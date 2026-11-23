#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. This body reads the cell once for the word copy, once more
 * into a callee-saved register before publishing the saved pointer, and writes
 * it twice around the helper call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared action-slot record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. Word 0x74 of
 * that record is a saved work-object pointer: the same main-executable cell is
 * written that way by
 * reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThenAdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C
 * and read back out of word 0x74 by
 * clearRecordPoolBytes0To2PublishRecordPoseSetGlobalByte80145988ToOneCopy16HalfwordsSettingBit15RequestResource1ThenAdvanceHandlerIndexBmagicMagic08103_801EED44,
 * both lifted exact in emi/bmagic/magic081/03, and this body reads the same
 * word here. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Shared resource request helper at 0x8014D6B8, outside this overlay's image.
 * Plchar lifts and the bmagic siblings named above call it as the resource
 * request helper and keep no result, so its role stays address-anchored here.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added. */
extern void func_8014D6B8(u32 flag);

/* Returns the whole 0x801EF2A8 record pool of this overlay to the free state.
 * Already lifted exact in this target as
 * src/bof3/bmagic/clearAllRecordActiveBytesBmagicMagic11603_801EEFF4.c. */
void clearAllRecordActiveBytesBmagicMagic11603_801EEFF4(void);

/* @source 0x801EEC80
 * @behavior Entry stage 0 of this overlay: word 0 of the four-word pointer run
 *           at 0x801EEC04 (0x801EEC80, 0x801EED2C, 0x801EEDB0, 0x801EEE34),
 *           which 0x801EEC14 copies into its frame and calls through unsigned
 *           work byte 0x01, so no jal and no table word inside this payload
 *           names this span. It takes the shared record through the pointer
 *           cell 0x801EC2E0, follows that record's word 0x74 to the saved
 *           work-object pointer, and copies the three words at 0x34, 0x38 and
 *           0x3C of that object into the current work object at the same three
 *           offsets, adding 0x01000000 to the 0x3C word. Each of the three
 *           stores re-reads the record word 0x74, so the walk is one pointer
 *           load per store rather than one saved pointer. It then returns the
 *           whole 0x801EF2A8 record pool to the free state through
 *           0x801EEFF4, re-reads the record cell 0x801EC2E0, keeps the
 *           scratchpad work-object pointer in a callee-saved register,
 *           publishes the saved work-object pointer through 0x1F800044, asks
 *           the shared helper 0x8014D6B8 for resource 1 while that pointer is
 *           current, restores 0x1F800044, and increments byte 0x01 of the work
 *           object, the step selector the dispatcher reads, so the next frame
 *           runs step 1. Takes no arguments and returns nothing; the 0x18-byte
 *           frame keeps ra at 0x14 and the saved cell value at 0x10 across the
 *           two calls. The role of the 0x34/0x38/0x3C triple, the meaning of
 *           the 0x01000000 bias and which resource 1 is stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySavedWorkPointerWords34To3CClearAllRecordActiveBytesRequestResource1AndAdvanceWorkByte1BmagicMagic11603_801EEC80(void) {
    u8 *record;
    u8 *work;
    u8 *saved;
    u8 *next;

    record = D_801EC2E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(*(u8 **)(record + 0x74) + 0x34);
    *(s32 *)(work + 0x38) = *(s32 *)(*(u8 **)(record + 0x74) + 0x38);
    *(s32 *)(work + 0x3C) = *(s32 *)(*(u8 **)(record + 0x74) + 0x3C) + 0x1000000;
    clearAllRecordActiveBytesBmagicMagic11603_801EEFF4();
    next = D_801EC2E0;
    saved = g_battle_work;
    g_battle_work = *(u8 **)(next + 0x74);
    func_8014D6B8(1);
    g_battle_work = saved;
    saved[1] = saved[1] + 1;
}
