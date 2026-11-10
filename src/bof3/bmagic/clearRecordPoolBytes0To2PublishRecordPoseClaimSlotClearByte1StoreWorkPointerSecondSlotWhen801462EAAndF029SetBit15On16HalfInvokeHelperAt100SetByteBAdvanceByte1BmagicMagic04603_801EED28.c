#include "base/types.h"

/* Record pool of this overlay at 0x801EFF3C: 48 records of stride 0x78 whose
 * byte 0 is the flags byte that reserveFirstFreeRecordBmagicMagic04603_801EFED0
 * tested for bit 0 and set on reservation, and whose bytes 0x00, 0x01 and 0x02
 * this body clears for every record. The pool base carries the map row of the
 * reservation lift; the two bytes that follow it are bound by the
 * address-anchored rows of this target generated symbol table. */
extern u8 magic04603RecordFlags[]; /* @source 0x801EFF3C @kind data */
extern u8 D_801EFF3D[]; /* @source 0x801EFF3D @kind data */
extern u8 D_801EFF3E[]; /* @source 0x801EFF3E @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target map
 * row g_battle_work as the bmagic siblings of other targets declare it. The
 * body takes six separate reads of the cell - one for the record byte 0x08
 * copy, one for the record word triplet, one per queued action slot, one for
 * work byte 0x0B and one for the handler index - because a store through the
 * loaded pointer may alias the cell, so the original re-materialises the
 * address with its own lui / lw pair at each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Queued work record pointer in main RAM, outside this overlay image. The
 * sibling lift copyQueuedWorkToScratchAndRun copies record byte 0x08 and the
 * word triplet 0x34, 0x38 and 0x3C of this same cell into the scratchpad work
 * object, so the body repeats an established copy rather than a new role. Splat
 * binds the address through the generated symbol table row of this target. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Enemy work record pointer in main RAM that the main program publishes and the
 * battle overlays read back. Byte 0xF0 of that record is compared here against
 * 0x29; nothing else in this target reads the byte, so its role stays
 * address-anchored. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared main-RAM byte used here only as a two-state selector: the body tests it
 * for zero before the second queued action slot is claimed. Its role is not
 * established by this target, so the name keeps the address anchor. */
extern u8 D_801462EA; /* @source 0x801462EA @kind data */

/* Byte 1 and word 0x74 of the 0x78-byte-stride queued action slot store based at
 * 0x801EC330 in main RAM. The slot allocator func_801E590C hands back the index
 * of the entry it claimed, and this body reaches each field through its own flat
 * symbol - one shared scaled index registered against a base named per field -
 * exactly as the sibling lift
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndA
 * dvanceHandlerIndexBmagicMagic08103_801EEEDC reached the same pair. Byte 1 is
 * cleared for the first slot and set to 1 for the second; its role beyond those
 * two written values is not established. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-RAM byte outside this overlay image. The sibling lift
 * clearRecordPoolBytes0To2PublishRecordPoseSetGlobalByte80145988ToOneCopy16Half
 * wordsSettingBit15RequestResource1ThenAdvanceHandlerIndexBmagicMagic08103_801E
 * ED44 promoted 1 into this address at the same point of the same stage, and
 * several lifted targets in this tree describe it as a stage serial; nothing in
 * this overlay establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Queued action slot allocator of the shared main executable at 0x801E590C,
 * outside this overlay image. Lifted from 0x801E590C in
 * src/bof3/battle/func_801E590C.c, which scans the first 0x30 entries of the
 * 0x801EC330 store for the first entry whose byte 0 has bit 0 clear, sets that
 * bit, stores its two arguments into slot bytes 6 and 5 and returns the entry
 * index, or 0xFF when every entry is taken. This body calls it with selector 1
 * and 0x32 twice and scales the zero-extended low byte of each answer by the
 * 0x78-byte stride. Bound by the generated function symbol row of this target. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image, called with
 * 0x100 and with no part of an answer used. The bmagic siblings of other targets
 * call it with 0x200-codes and 0x100 the same way, so only its effect is
 * established and its role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EED28
 * @behavior Init stage 0 of this overlay: entry 0 of the two-word handler table
 *           the frame dispatcher func_801EEC28 builds on its own frame and calls
 *           through work byte 0x1 scaled by four, so no jal in the payload names
 *           it. It clears bytes 0x00, 0x01 and 0x02 of all 48 records of the
 *           0x78-byte-stride pool at 0x801EFF3C, computing the stride from the
 *           masked byte counter and testing that counter against 0x30 at the
 *           loop bottom. It then publishes the queued work record pose into the
 *           scratchpad work object at the cell 0x1F800044: work byte 0x08
 *           receives record byte 0x08 and work words 0x34, 0x38 and 0x3C receive
 *           the record words at the same offsets, with the record and the work
 *           object read back once for byte 0x08 and again for the word triplet.
 *           It claims one entry of the queued action slot store through the
 *           shared allocator func_801E590C, called with selector 1 and 0x32, and
 *           takes the zero-extended low byte of the answer as the entry index;
 *           the index scaled by the 0x78-byte stride reaches slot byte 0x01,
 *           which is cleared, and slot word 0x74, which receives the work-object
 *           pointer read from the scratchpad cell. When the shared main-RAM byte
 *           0x801462EA is not zero and byte 0xF0 of the enemy work record reached
 *           through the cell 0x801EB4E8 equals 0x29, it claims a second entry the
 *           same way, sets that slot byte 0x01 to 1 and stores the same work
 *           pointer in slot word 0x74. Either path then publishes 1 into the
 *           shared main-RAM byte 0x80145988, copies the sixteen unsigned
 *           halfwords of the table at 0x80036C00 into the table at 0x8003AC00
 *           with bit 15 set on every entry, restores entry 0 of the destination
 *           to the source value without that bit, invokes the main-executable
 *           helper func_8015D71C with 0x100, sets work byte 0x0B to 1 and
 *           increments work byte 0x1, the handler index this overlay dispatches
 *           on, so the next frame reaches entry 1 of the same run. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps ra at
 *           0x10 across the calls. The two table addresses, the roles of pool
 *           bytes 0x01 and 0x02, of slot byte 0x01 and of the shared bytes
 *           0x801462EA and 0x80145988 are not established here, so they stay
 *           address-anchored.
 * @status partial
 * @match 97.54
 * @residual The first claim loads the work object through $a0 instead of $v0
 *           and stores that register to slot word 0x74. The pool-clear loop
 *           matches after capturing the old byte index separately from the
 *           offset and incrementing the index in a single-execution scope.
 */
void clearRecordPoolBytes0To2PublishRecordPoseClaimSlotClearByte1StoreWorkPointerSecondSlotWhen801462EAAndF029SetBit15On16HalfInvokeHelperAt100SetByteBAdvanceByte1BmagicMagic04603_801EED28(
    void) {
  u8 index;
  u32 offset;
  u8 *published;
  u8 *record;
  u8 *work;
  u8 publishedValue;

  for (index = 0; index < 0x30;) {
    u32 priorIndex = index;
    do {
      index++;
    } while (0);
    offset = priorIndex * 0x78u;
    magic04603RecordFlags[offset] = 0;
    D_801EFF3D[offset] = 0;
    D_801EFF3E[offset] = 0;
  }

  g_battle_work[8] = D_801EB4E0[8];
  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
  *(s32 *)(work + 0x3C) = *(s32 *)(record + 0x3C);

  offset = (u8)func_801E590C(1u, 0x32u) * 0x78u;
  published = g_battle_work;
  D_801EC331[offset] = 0;
  *(u32 *)&D_801EC3A4[offset] = (u32)published;

  if ((D_801462EA != 0) && (D_801EB4E8[0xF0] == 0x29)) {
    offset = (u8)func_801E590C(1u, 0x32u) * 0x78u;
    published = g_battle_work;
    D_801EC331[offset] = 1;
    *(u32 *)&D_801EC3A4[offset] = (u32)published;
  }
  publishedValue = 1;
  D_80145988 = publishedValue;

  for (index = 0; index < 0x10; index++) {
    ((u16 *)0x8003AC00)[index] = ((u16 *)0x80036C00)[index] | 0x8000;
  }
  ((u16 *)0x8003AC00)[0] = ((u16 *)0x80036C00)[0];

  func_8015D71C(0x100);
  g_battle_work[0xB] = 1;
  g_battle_work[1]++;
}
