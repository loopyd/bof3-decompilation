#include "bof3/bof3.h"

/* Record pool of this overlay: 36 records of stride 0x78 starting at
 * 0x801EFEF4, so the pool ends at 0x801F0FD4. The exact reservation lift
 * reserveFirstFreeRecordBmagicMagic05803_801EFE38 walks the same run through
 * the base symbol and the still-assembled dispatcher sweep at 0x801EEC90
 * indexes it the same way, so that name is kept. This body clears the first
 * three bytes of every record and the original re-materializes the pool
 * address for each store, so each byte is addressed through its own base. */
extern u8 D_801EFEF4[]; /* @source 0x801EFEF4 @kind data */
extern u8 D_801EFEF5[]; /* @source 0x801EFEF5 @kind data */
extern u8 D_801EFEF6[]; /* @source 0x801EFEF6 @kind data */

/* Byte table that begins where the record pool ends: eleven bytes at
 * 0x801F0FD4, all cleared by this body. Nothing else in the payload reads the
 * span, so its role stays address-anchored. */
extern u8 D_801F0FD4[]; /* @source 0x801F0FD4 @kind data */

/* Battle record pointer published by the main executable at 0x801EB4E0,
 * outside this overlay image and already bound by this target's generated
 * undefined_syms_auto.txt. The exact sibling
 * seedWorkFromRecordCopyByte8AndWordTripletSetByteATo16AndWordCTo8ThenAdvanceWorkByte2BmagicMagic05803_801EF008
 * copies the same record byte 0x08 and the same record word triplet into the
 * work object, so this is a repeat of an established copy. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target map
 * row g_battle_work as the other exact lifts of this target declare it. A
 * store through the loaded pointer may alias the cell, so the original
 * re-materializes the address with its own lui and offset load at every read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Byte 1 and word 0x74 of the 0x78-byte-stride queued action slot store based
 * at 0x801EC330 in main RAM, reached here through one flat symbol per field
 * exactly as the sibling init-stage lifts of the other bmagic targets reach
 * them. The slot allocator func_801E590C hands back the index of the entry it
 * claimed; byte 1 and word 0x74 are the only two fields this body writes. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Queued action slot allocator of the shared main executable at 0x801E590C,
 * outside this overlay image and lifted at src/bof3/battle/func_801E590C.c:
 * it scans the first 0x30 entries of the D_801EC330 store for the first entry
 * whose byte 0 has bit 0 clear, sets that bit, stores its two arguments into
 * slot bytes 6 and 5 and returns the entry index, or 0xFF when every entry is
 * taken. This body calls it once with 1 and 0x3F and scales the low byte of
 * the answer by the 0x78-byte stride. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image, called
 * with 0x100 and with no part of an answer used. The bmagic siblings of other
 * targets call the same helper the same way, so only its call is established
 * and its role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EED30
 * @behavior Init stage 0 of this overlay: word 0 of the nine-word code pointer
 *           run the payload header holds at 0x801EEC04, which the frame
 *           dispatcher func_801EEC28 copies to its own frame and calls through
 *           work byte 0x01 scaled by four, so no jal in the payload names it.
 *           It clears bytes 0x00, 0x01 and 0x02 of all 36 records of the
 *           0x78-byte-stride pool at 0x801EFEF4, computing the stride from the
 *           masked byte counter and testing that counter against 0x24 at the
 *           loop bottom, then clears the eleven bytes of the table at
 *           0x801F0FD4. It publishes the battle record pose into the scratchpad
 *           work object at the cell 0x1F800044: work byte 0x08 receives record
 *           byte 0x08 and work words 0x34, 0x38 and 0x3C receive the record
 *           words at the same offsets, with the record and the work object read
 *           back once for byte 0x08 and again for the word triplet. Work byte
 *           0x0B is cleared between the word 0x38 and word 0x3C stores, the cell
 *           is read once more after that clear and work byte 0x01 is advanced by
 *           one. It then claims one entry of the queued action slot store
 *           through the shared allocator func_801E590C, called with the selector
 *           1 and 0x3F, takes the zero-extended low byte of the answer as the
 *           entry index, and uses the index scaled by the 0x78-byte stride to
 *           clear slot byte 0x01. The cell is read again for the work byte 0x0B
 *           increment before the slot word 0x74 store is emitted, that store
 *           takes the work-object pointer read from the scratchpad cell, work
 *           byte 0x0B is advanced by one, and the main-executable helper
 *           func_8015D71C is invoked with 0x100. It keeps the return address at
 *           0x10 in a 0x18-byte frame, takes no arguments and returns nothing.
 *           The role of the eleven-byte table, of slot byte 0x01, of work bytes
 *           0x0B and 0x01 and of the helper is not established here, so those
 *           stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordPoolBytes0To2AndByteTableThenPublishRecordPoseClearWorkByteBAdvanceWorkByte1ClaimSlotAt3FClearSlotByte1StoreWorkPointerIncrementWorkByteBInvokeHelperAt100BmagicMagic05803_801EED30(
    void) {
  u8 index;
  u32 offset;
  u8 *record;
  u8 *work;
  u8 *next;
  u8 *published;
  u8 *queued;
  u32 value;

  for (index = 0; index < 0x24;) {
    u32 priorIndex = index;
    do {
      index++;
    } while (0);
    offset = priorIndex * 0x78u;
    D_801EFEF4[offset] = 0;
    D_801EFEF5[offset] = 0;
    D_801EFEF6[offset] = 0;
  }

  for (index = 0; index < 0xB; index++) {
    D_801F0FD4[index] = 0;
  }

  g_battle_work[8] = D_801EB4E0[8];
  record = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  value = *(u32 *)(record + 0x3C);
  work[0x0B] = 0;
  next = g_battle_work;
  *(u32 *)(work + 0x3C) = value;
  next[1]++;

  index = (u8)func_801E590C(1u, 0x3Fu);
  published = g_battle_work;
  D_801EC331[index * 0x78u] = 0;
  queued = g_battle_work;
  *(u32 *)&D_801EC3A4[index * 0x78u] = (u32)published;
  queued[0x0B]++;
  func_8015D71C(0x100);
}
