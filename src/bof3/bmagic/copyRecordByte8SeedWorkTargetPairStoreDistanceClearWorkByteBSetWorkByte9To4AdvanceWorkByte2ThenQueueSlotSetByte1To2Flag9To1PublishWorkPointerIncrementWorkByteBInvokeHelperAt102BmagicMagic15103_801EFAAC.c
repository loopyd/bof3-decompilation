#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original re-materialises the cell with its own lui / lw
 * pair at every store group: once before the 0x18-byte frame is created, again
 * for the target-pair stores, again after the distance call, twice for the
 * countdown and state bytes, once for the action-slot pointer word and once more
 * for the closing counter increment. Each of those sites keeps its own plain
 * cell read, because a store through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pointer at 0x8014638C in main RAM. This lift reads only byte 0x08 of
 * the record it points to and copies it into work byte 0x08; its role is not
 * proven, so the name stays address-anchored, exactly as the exact siblings on
 * this target declare it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Signed main-RAM words installed as the work-object target pair at 0x34 and
 * 0x38. The battle lift func_801DF3F8 stores the same two words into the same
 * two work words on its "place" branch, and the distance helper below consumes
 * the pair, so they are read as signed words here; neither role is proven, so
 * both names stay address-anchored. */
extern s32 D_80143F24; /* @source 0x80143F24 @kind unknown */
extern s32 D_80143F28; /* @source 0x80143F28 @kind unknown */

/* Fields of the shared 0x78-byte action-slot store pool at 0x801EC330. The
 * original reaches each field through its own flat symbol (lui $at,%hi(SYM) +
 * addu at,at,index*0x78 + sb|sw %lo(SYM)(at)) rather than a shared record base,
 * so every addressed field keeps its own byte-viewed symbol: entry byte +0x01,
 * entry byte +0x09 (the step counter the sibling lifts on this family write
 * through the same symbol) and the pointer word at +0x74. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable helpers, all outside this overlay image. The distance
 * helper's two-argument signed-halfword prototype is the one the scenario,
 * world, battle and bmagic lifts declare for this address. The slot allocator at
 * 0x801E590C returns the index of the entry it claimed, and this body does not
 * test its 0xFF "pool full" answer. func_8015D71C takes one literal and returns
 * nothing. */
extern s16 func_8015477C(s32 arg0, s32 arg1); /* @source 0x8015477C */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EFAAC
 * @behavior Entry index 0 of the six-word handler run at 0x801EEC38, which the
 *           dispatcher dispatchWorkByte2HandlerBmagicMagic15103_801EFA34 selects
 *           with the scratchpad work object's byte 0x02. The scratchpad
 *           work-object pointer cell at 0x1F800044 is read before this
 *           function's 0x18-byte frame is created, and work byte 0x08 receives
 *           byte 0x08 of the record the pointer at 0x8014638C holds. On the next
 *           cell read the work word at 0x34 receives the main-RAM word
 *           0x80143F24, the work word at 0x38 receives 0x80143F28 through a
 *           signed local, and the shared distance helper func_8015477C is called
 *           with the work word 0x34 read back from the frame together with that
 *           local, its signed-halfword answer landing in work halfword 0x3E. Both
 *           the read-back and the helper's second argument are the original's:
 *           the store to 0x38 sits between the store and the load of 0x34, so
 *           that load cannot reuse the stored register. The signed local also
 *           has to hold the 0x38 word before the pair stores are emitted - that
 *           independent load is what the original schedules into 0x34's store
 *           delay, while declaring it after the 0x34 store measured a nop there
 *           and four bytes over the original. The same cell read clears
 *           work byte 0x0B; the body then re-reads the cell for work byte 0x09
 *           (4) and once more to increment work byte 0x02, which advances this
 *           overlay's work byte 2 sequence. It claims an action slot with
 *           func_801E590C(1, 0x44) and, from the answer taken at byte width and
 *           scaled by the 0x78-byte slot-store stride, stamps 2 into entry byte
 *           +0x01, 1 into entry byte +0x09 and the work-object pointer into the
 *           pointer word at +0x74, that pointer being the cell read taken ahead
 *           of the two byte stamps. The closing group increments work byte 0x0B
 *           through its own cell read, taken before the pointer-word store so
 *           that store's delay slot carries the byte load, and invokes
 *           func_8015D71C with 0x102. Frame 0x18 with $ra
 *           at 0x10, because of the three calls; takes no arguments, returns
 *           nothing and reads or writes no other state. The roles of 0x8014638C
 *           and of the two 0x80143F2x words beyond their position-pair and
 *           byte-copy use remain unestablished, so those bindings stay
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordByte8SeedWorkTargetPairStoreDistanceClearWorkByteBSetWorkByte9To4AdvanceWorkByte2ThenQueueSlotSetByte1To2Flag9To1PublishWorkPointerIncrementWorkByteBInvokeHelperAt102BmagicMagic15103_801EFAAC(
    void) {
  u8 *work;
  u8 *slotWork;
  u8 *armWork;
  s32 target;
  s16 distance;
  u32 offset;

  work = g_battle_work;
  work[8] = D_8014638C[8];
  work = g_battle_work;
  target = D_80143F28;
  *(u32 *)(work + 0x34) = (u32)D_80143F24;
  *(u32 *)(work + 0x38) = (u32)target;
  distance = func_8015477C((s32) * (u32 *)(work + 0x34), target);
  work = g_battle_work;
  *(u16 *)(work + 0x3E) = (u16)distance;
  work[0x0B] = 0;
  g_battle_work[9] = 4;
  g_battle_work[2]++;
  offset = (u8)func_801E590C(1u, 0x44u) * 0x78;
  slotWork = g_battle_work;
  D_801EC331[offset] = 2;
  D_801EC339[offset] = 1;
  armWork = g_battle_work;
  *(u32 *)&D_801EC3A4[offset] = (u32)slotWork;
  armWork[0x0B]++;
  func_8015D71C(0x102);
}
