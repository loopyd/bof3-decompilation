#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work, the name the exact bmagic lifts of this family use for the
 * same main-executable cell. The original materialises the cell with its own
 * `lui` / `lw` pair at each read - the copy of record byte 8, the copy of the
 * record word triplet, the pointer published into the claimed slot and each of
 * the two closing byte stores - because the store that follows a read may
 * republish the pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pointer cell at 0x801EB4E0 of the main executable, bound by this
 * target's generated undefined_syms_auto.txt. It holds the address of the record
 * whose byte 8 and words 0x34, 0x38 and 0x3C this step copies into the work
 * object. Its role beyond being the copy source stays address-anchored, and the
 * exact bmagic sibling
 * copyRecordByte8AndWordTripletPlus2000000InvokeHelpersSetByte9ToFByteATo3ClearByteBAndAdvanceWorkByte2BmagicMagic06803_801EF7C4
 * reads the same cell with this spelling. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Byte 1 and word 0x74 of the 0x78-byte-stride queued action-slot store based at
 * 0x801EC330 in main RAM, outside this overlay image. The slot allocator
 * func_801E590C returns the index of the entry it claimed and this body scales
 * that index by the 0x78-byte stride once, then reaches both fields through their
 * own flat symbol - `lui $at,%hi(SYM)`, `addu $at,$at,index*0x78`, access
 * `%lo(SYM)($at)` - with the scaled index kept in one register across the two
 * accesses. The slot byte 1 is cleared here; the value 0 is the only one this
 * overlay writes into it, so its role stays address-anchored. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Queued action-slot allocator of the shared main executable at 0x801E590C,
 * outside this overlay image, bound by this target's generated
 * undefined_funcs_auto.txt. This body calls it with selector 1 and 0x1C, keeps
 * the zero-extended low byte of the answer as the slot index and does not check
 * it for the 0xFF failure value. The exact bmagic lifts of this family call the
 * same address with the same two-argument shape. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801EEC88
 * @behavior First step of this overlay's work-byte-1 handler run: the payload
 *           holds no jal word for this address, only entry 0 of the five code
 *           pointers at 0x801EEC04, which the dispatcher at 0x801EEC18 copies
 *           into its frame and invokes with work byte 1 as the index, so this
 *           body runs while work byte 1 is zero and its own closing increment
 *           selects entry 1 (0x801EED50) of the same run for the next frame.
 *           Copies byte 8 of the record published by the pointer cell 0x801EB4E0
 *           into work byte 8, re-reads both pointer cells, and copies the record
 *           words 0x34, 0x38 and 0x3C into the same three work words without a
 *           bias. Claims one queued action slot through the allocator at
 *           0x801E590C with the arguments 1 and 0x1C, truncates the returned
 *           index to a byte and scales it by the 0x78-byte slot stride, clears
 *           that slot's byte 1 and publishes a fresh read of the scratchpad cell
 *           0x1F800044 into the slot's word 0x74. Sets work byte 0xB to the
 *           literal 1 and increments work byte 1 through its own read of the
 *           cell. The 0x18-byte frame keeps only $ra at 0x10; the two pointer
 *           cells are read once before the frame reservation and every later read
 *           sits at its own source site because the byte stores in between may
 *           republish them. Takes no arguments and returns nothing. Statement
 *           order, operand forms, offset spelling and declaration style follow
 *           the exact siblings
 *           clearRecordPoolBytes0To2PublishRecordPoseClaimSlotClearByte1StoreWorkPointerSecondSlotWhen801462EAAndF029SetBit15On16HalfInvokeHelperAt100SetByteBAdvanceByte1BmagicMagic04603_801EED28
 *           (the same record byte 8 and word triplet copy into the work object,
 *           the same allocator call, slot byte 1 clear and word 0x74 pointer
 *           store through one scaled index, and the same closing byte 0xB store
 *           followed by the byte 1 increment) and
 *           copyRecordByte8AndWordTripletPlus2000000InvokeHelpersSetByte9ToFByteATo3ClearByteBAndAdvanceWorkByte2BmagicMagic06803_801EF7C4
 *           (the same two-pair local split, the first pair in $v0 and $v1 and the
 *           second in $v1 and $a0). This body's own literals 1 and 0x1C, its
 *           record cell, its band offsets and its slot stride are written from
 *           the original bytes. A payload scan of the 398 emi images under
 *           out/binaries found no byte-identical copy of this 0xC8-byte span: the
 *           closest span (emi/bmagic/magic040/03.bin at offset 1140) differs in
 *           82 of its 200 bytes, so no clone seeded this body. The roles of
 *           record byte 8, of record words 0x34/0x38/0x3C beyond being copied,
 *           of work bytes 0xB and 1 beyond the literals stored here, of slot byte
 *           1 and of the allocator's 0x1C selector stay unestablished.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordByte8AndWordTripletClaimSlot1CClearSlotByte1StoreWorkPointerSetByteBTo1AndAdvanceByte1BmagicMagic06803_801EEC88(
    void) {
  u32 offset;
  u8 *published;
  u8 *record;
  u8 *work;

  g_battle_work[8] = D_801EB4E0[8];
  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
  *(s32 *)(work + 0x3C) = *(s32 *)(record + 0x3C);

  offset = (u8)func_801E590C(1u, 0x1Cu) * 0x78u;
  published = g_battle_work;
  D_801EC331[offset] = 0;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;

  g_battle_work[0xB] = 1;
  g_battle_work[1]++;
}
