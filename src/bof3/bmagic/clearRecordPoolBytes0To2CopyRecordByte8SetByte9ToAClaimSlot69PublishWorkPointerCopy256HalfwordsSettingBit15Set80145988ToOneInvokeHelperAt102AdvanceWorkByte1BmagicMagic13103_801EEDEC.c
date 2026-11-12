#include "bof3/bof3.h"

/* Record pool of this overlay at 0x801F1CA0: 48 records of stride 0x2C, the
 * 0x840-byte span that ends exactly at the record pointer cell 0x801F24E0. The
 * target map binds the flags byte of every record through
 * magic13103RecordFlags, and the sibling lift
 * reserveFirstFreeRecordBmagicMagic13103_801F1A58 walked the same pool with the
 * same 0x30 count and 0x2C stride. The original addresses byte 0 and the two
 * bytes that follow it with their own `lui` / `addu` / `sb` triple, so the two
 * trailing bytes keep the address-anchored names the payload's own asm used and
 * the generated undefined_syms_auto.txt binds them. */
extern u8 magic13103RecordFlags[]; /* @source 0x801F1CA0 @kind data */
extern u8 D_801F1CA1[]; /* @source 0x801F1CA1 @kind data */
extern u8 D_801F1CA2[]; /* @source 0x801F1CA2 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. This body loads the cell once at each site that uses it -
 * the record byte copy, work byte 0xB, work byte 9, work byte 0xA, the
 * handler-index increment, the published work pointer and the queue count -
 * because the stores through the loaded pointer may alias the cell, so the
 * original re-materializes the address with its own `lui` / `lw` pair at each
 * read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image
 * and bound by this target's map row. Only its byte 0x8 is read here; the
 * target's other sources declare the cell word-typed and derive the byte view,
 * which is the form kept here. */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Action-slot store in main RAM at 0x801EC330 plus the 0x78-byte stride. The
 * original reaches each field through its own flat symbol - `lui $at,%hi(SYM)`,
 * `addu $at,$at,offset`, store `%lo(SYM)($at)` - with the scaled allocator
 * answer in the one register shared by both stores, exactly as the sibling lift
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndA
 * dvanceHandlerIndexBmagicMagic08103_801EEEDC declared them; both addresses
 * come from this target's generated undefined_syms_auto.txt. The roles of slot
 * byte 0x01 and of slot word 0x74 are not established beyond the stores here,
 * so both stay address-anchored. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-RAM byte outside this overlay's image, bound by this target's
 * generated undefined_syms_auto.txt row. Nothing in this target establishes its
 * role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Main-executable helper at 0x800AFD9C, outside this overlay image and bound
 * through this target's generated undefined_funcs_auto.txt. Called with no
 * argument register set, the same zero-argument form the sibling lift
 * clearRecordPoolBytes0To2InvokeSharedHelperAddC00000ToWorkWord3CCopy256Halfwo
 * rdsSettingBit15SetByte80145988ToOneCopyFirstHalfwordInvokeHelperAt100ClearWor
 * kBytes9AndBAdvanceWorkByte1BmagicMagic16903_801EED8C recorded; this body uses
 * no part of an answer, so only its effect is established and its role stays
 * address-anchored. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */

/* Action-slot allocator of the shared main executable at 0x801E590C, outside
 * this overlay image and bound through this target's generated
 * undefined_funcs_auto.txt. This body passes request class 1 and code 0x69 and
 * uses the zero-extended low byte of the answer as the index of the slot it
 * claimed. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image and bound
 * through this target's generated undefined_funcs_auto.txt. The bmagic siblings
 * call it with one code and use no part of an answer, so only its effect is
 * established and its role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EEDEC
 * @behavior Entry stage 0 of this overlay: word 0 of the eight-word code
 *           pointer run at 0x801EEC04, whose words the frame dispatcher
 *           func_801EEC24 copies onto its own stack and calls through the
 *           work-object byte 0x1 scaled by four, so no jal in this payload names
 *           the body. It clears bytes 0x00, 0x01 and 0x02 of all 0x30 records
 *           of the 0x2C-byte-stride pool at 0x801F1CA0 with one sb per byte,
 *           computing the stride from the masked index and testing the
 *           incremented index against 0x30 at the loop bottom. It then copies
 *           byte 0x8 of the shared battle work record reached through the
 *           pointer cell 0x801EB4E0 into work byte 0x8 and calls the shared
 *           helper at 0x800AFD9C, the copy filling the call delay slot; clears
 *           work byte 0xB, sets work byte 0x9 to 0xA, clears work byte 0xA and
 *           increments work byte 0x1, the handler index this overlay dispatches
 *           on, so the next frame runs word 1 of the run at 0x801EEF64. It
 *           claims one action slot through the shared allocator
 *           func_801E590C(1, 0x69), stores 1 into that slot's byte 0x01,
 *           publishes the work-object pointer read from the scratchpad cell
 *           0x1F800044 into slot word 0x74 and increments work byte 0xB, the
 *           countdown work byte 9 counts down. It then copies the 0x100 unsigned
 *           halfwords of the table at 0x80036C00 into the table at 0x8003AC00
 *           with bit 15 set on every entry, stores 1 into the shared main-RAM
 *           byte 0x80145988 and invokes the main-executable helper func_8015D71C
 *           with 0x102. Takes no arguments and returns nothing; the 0x18-byte
 *           frame keeps only ra at 0x10 across the two calls. The roles of pool
 *           bytes 0x01 and 0x02, of slot byte 0x01, of the two halfword tables,
 *           of the allocator's code 0x69 and of helper code 0x102 remain
 *           unestablished, so those stay literal or address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordPoolBytes0To2CopyRecordByte8SetByte9ToAClaimSlot69PublishWorkPointerCopy256HalfwordsSettingBit15Set80145988ToOneInvokeHelperAt102AdvanceWorkByte1BmagicMagic13103_801EEDEC(
    void) {
  u16 index;
  u32 offset;
  u8 slot;
  u32 slotOffset;
  u8 *published;
  u8 *counter;

  for (index = 0; index < 0x30; index++) {
    offset = index * 0x2C;
    magic13103RecordFlags[offset] = 0;
    D_801F1CA1[offset] = 0;
    D_801F1CA2[offset] = 0;
  }

  g_battle_work[8] = ((u8 *)record801EB4E0)[8];
  func_800AFD9C();

  g_battle_work[0xB] = 0;
  g_battle_work[9] = 0xA;
  g_battle_work[0xA] = 0;
  g_battle_work[1]++;

  slot = (u8)func_801E590C(1u, 0x69u);
  slotOffset = slot * 0x78;
  published = g_battle_work;
  D_801EC331[slotOffset] = 1;
  counter = g_battle_work;
  *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
  counter[0xB]++;

  for (index = 0; index < 0x100; index++) {
    PSX_PTR(u16, 0x8003AC00u)[index] =
        PSX_PTR(u16, 0x80036C00u)[index] | 0x8000;
  }
  D_80145988 = 1;
  func_8015D71C(0x102);
}
