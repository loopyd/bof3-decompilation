#include "bof3/bof3.h"

/* Shared battle record pointer cell at 0x801EB4E0 in main executable RAM,
 * outside this overlay image. The original loads it once with a single
 * `lui $a0,%hi` / `lw $a0,%lo` pair and reads its three words through that one
 * register, so the body keeps the cell value in a local. The sibling
 * seedWorkFromRecordQueueSelectedBattlerSlot49PublishRecordBytesRaiseFlag40AndA
 * dvanceWorkByte1BmagicMagic00100_801EECC4 binds the same address and the
 * generated undefined_syms_auto.txt row of this target already names it. Its
 * field roles stay address-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work like the other lifts of this payload declare it. The body
 * re-reads the cell at every site - the three byte stores and the byte 0x1
 * increment before the slot queue, the counted and published pointers of every
 * claimed slot - because the stores through the loaded pointer may alias the
 * cell, so the original re-materializes the address with its own `lui` / `lw`
 * pair at each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Action-slot store at 0x801EC330 in main RAM: 0x78-byte stride, the pool the
 * sibling queueActionSlotSetByte4Byte9AndByteAAndPublishWorkPointerThenCountWork
 * ByteBAndIncrementWorkByte1BmagicMagic08803_801EF1AC documents. The original
 * reaches each field through its own flat symbol - `lui $at,%hi(SYM)`,
 * `addu $at,$at,index*0x78`, store `%lo(SYM)($at)` - with the scaled index in
 * one register shared by every store of the iteration, so each addressed field
 * keeps its own address-anchored name: the byte at +0x01, the byte at +0x09 and
 * the work-object pointer word at +0x74. This body clears slot byte 0x01 of the
 * eight queued slots and stamps slot byte 0x09 with the slot ordinal's
 * eight-step value plus one, then sets slot byte 0x01 of the final slot. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Action-slot allocator of the shared main executable at 0x801E590C, outside
 * this overlay image and bound through this target's generated
 * undefined_funcs_auto.txt. This body passes selector 1 and sub-selector 0x3D
 * and uses the zero-extended low byte of the answer as the index it scales by
 * the 0x78-byte stride, the same two-step use the sibling lifts of this module
 * record for this allocator. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image and bound
 * through this target's generated undefined_funcs_auto.txt. The bmagic siblings
 * call it with one code and use no part of an answer, so only its effect is
 * established and its role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EECB0
 * @behavior Entry 0 of the six-word code pointer run at 0x801EEC04, which the
 *           frame entry func_801EEC38 copies and calls with no arguments. It
 *           copies words 0x34, 0x38 and 0x3C from the record pointer published
 *           at 0x801EB4E0 into the scratchpad work object, setting work byte
 *           0xB to zero between the last word read and that word's store. It
 *           then re-reads the work-object cell for each of three further
 *           stores: work byte 0x9 set to 0xA, work byte 0xA cleared to zero and
 *           work byte 0x1 incremented, the state-byte advance the dispatcher at
 *           0x801EF190 reads. A `do` loop of eight iterations then claims one
 *           action slot per iteration through func_801E590C(1, 0x3D), clears
 *           slot byte 0x01 at the index scaled by the 0x78-byte stride, stamps
 *           slot byte 0x09 with the iteration ordinal times eight plus one,
 *           publishes the work-object pointer read from the scratchpad cell
 *           into slot word 0x74 and increments work byte 0xB. After the loop it
 *           claims one more slot with the same allocator arguments, sets that
 *           slot byte 0x01 to one instead of clearing it, leaves slot byte 0x09
 *           untouched, publishes the work pointer into slot word 0x74 and
 *           increments work byte 0xB once more, then invokes the main-executable
 *           helper func_8015D71C with 0x100. Every iteration reads the
 *           work-object cell twice, once for the published pointer and once for
 *           the counted one; the slot tables and the scratchpad cell are
 *           disjoint, so sampling the cell across those slot stores preserves
 *           its value. The 0x18-byte frame holds $ra at
 *           0x14 and $s0 at 0x10; the loop ordinal is a byte local. It takes no
 *           arguments, returns nothing and reads or writes no other state.
 *           Selector 0x3D, helper code 0x100 and the record, slot and table
 *           field roles stay unestablished constants and offsets.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The published-first loop with the same pointer roles in the final claim
 * measured 82/89 instructions. Reading the counted pointer first improved that
 * to 86/89, with three register-only differences. Exchanging the two locals'
 * final-claim roles alone measured 85/89: it corrected the loop consumers but
 * exchanged the cell-load registers. Combining that real phase reuse with the
 * published-first loop matched all 89 instructions and 356 bytes. Both samples
 * and all memory effects remain; no register constraints or barriers are used.
 */
void seedWorkObjectFromBattleRecordTripletClearBytesBAndAQueueEightSlotsClearByte1StampByte9OrdinalStepPublishWorkPointerThenByte1SlotInvokeHelperAt100BmagicMagic04103_801EECB0(void) {
  u8 *record;
  u8 *work;
  u8 *slotWork;
  u8 *countWork;
  u8 i;
  u32 offset;
  u32 word3C;

  record = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)&work[0x34] = *(u32 *)&record[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&record[0x38];
  word3C = *(u32 *)&record[0x3C];
  work[0xB] = 0;
  *(u32 *)&work[0x3C] = word3C;
  g_battle_work[9] = 0xA;
  g_battle_work[0xA] = 0;
  g_battle_work[1]++;

  i = 0;
  do {
    offset = (u8)func_801E590C(1, 0x3D) * 0x78u;
    slotWork = g_battle_work;
    D_801EC331[offset] = 0;
    D_801EC339[offset] = i * 8 + 1;
    countWork = g_battle_work;
    *(u32 *)&D_801EC3A4[offset] = (u32)slotWork;
    countWork[0xB]++;
    i++;
  } while (i < 8);

  offset = (u8)func_801E590C(1, 0x3D) * 0x78u;
  countWork = g_battle_work;
  D_801EC331[offset] = 1;
  slotWork = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)countWork;
  slotWork[0xB]++;
  func_8015D71C(0x100);
}
