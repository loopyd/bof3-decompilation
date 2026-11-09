#include "bof3/bof3.h"

/* Shared per-frame counter in the main executable, already bound by this
 * target's generated undefined_syms_auto.txt and shared with the exact siblings
 * of this overlay, so no target-map row is needed. Only its low two bits are
 * read here: the every-fourth-frame gate the slot-queueing spawners of this
 * family also use. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Shared battle work record pointer at 0x801EB4E0, outside this overlay image
 * and already bound by the generated undefined_syms_auto.txt. Only its byte 2,
 * the shared work state byte, is read here, and the original re-reads the cell
 * for that one access. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings of this target use, so the symbolic address folds
 * into each load (lui $vN,0x1F80 / lw $vN,68($vN)). All four reads of this body
 * stay at their own sites: each one follows a store issued through a previously
 * loaded pointer, which may alias the cell, so the original re-materialises the
 * base before every read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: the state
 * byte at +0x01, byte +0x04 and the pointer word at +0x74 of one entry. Byte
 * +0x04 is write-only across the overlays lifted so far, so its role stays
 * offset-anchored. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable slot allocator, outside this payload. It returns the
 * index of the entry it claimed, or 0xFF when the pool holds no free entry; this
 * body checks no sentinel and stores through the returned index directly. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801EFF94
 * @behavior Two independent halves. The first runs only on the frames where the
 *           low two bits of the shared counter at 0x80143E6C are clear: it
 *           claims one action slot through func_801E590C(1, 0x44), scales the
 *           byte-truncated returned index by the 0x78-byte slot stride, and
 *           fills three fields of that entry - the state byte at +0x01 becomes
 *           1, the pointer word at +0x74 receives the scratchpad work-object
 *           pointer read from the cell at 0x1F800044, and byte +0x04 receives
 *           the low four bits of the work-object byte 0x04 through a second read
 *           of the cell. It then increments the work-object byte 0x0B and the
 *           work-object byte 0x04 with byte-width read-modify-writes, each on its
 *           own read of the cell, the second of which is the last store of the
 *           half. The second half reads the shared battle record pointer at
 *           0x801EB4E0 once and acts only while that record byte 0x02 equals 5:
 *           it decrements the work-object byte 0x0A and tests the truncated byte
 *           it just stored (andi 0xFF of the stored register, no second load of
 *           the cell); on expiry it re-reads the cell and increments the
 *           work-object state byte 0x02, which advances this overlay's work byte
 *           2 sequence. Both halves fall through to one shared epilogue, and a
 *           taken first half also reaches the second. Takes no arguments, returns
 *           nothing, and the 0x18-byte frame keeps only $ra at 0x10 because of
 *           the one call. The roles of work bytes 0x04, 0x0A and 0x0B, of the
 *           shared record byte 0x02 and of the slot byte 0x04 remain
 *           unestablished, so the name stays offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSlotEveryFourthTickSetByte1AndByte4PublishWorkPointerIncrementWorkBytesBAnd4ThenDecrementWorkByteAAndAdvanceWorkByte2WhenSharedRecordByte2Is5BmagicMagic15103_801EFF94(void) {
  u8 *queuedWork;
  u8 *slotWork;
  u32 slot;
  u32 offset;
  u8 countdown;

  if ((D_80143E6C & 3) == 0) {
    slot = (u8)func_801E590C(1u, 0x44u);
    offset = slot * 0x78;
    queuedWork = g_battle_work;
    D_801EC331[offset] = 1;
    slotWork = g_battle_work;
    *(u32 *)&D_801EC3A4[offset] = (u32)queuedWork;
    D_801EC334[offset] = slotWork[4] & 0xF;
    g_battle_work[0xB]++;
    g_battle_work[4]++;
  }
  if (D_801EB4E0[2] == 5) {
    countdown = g_battle_work[0xA] - 1;
    g_battle_work[0xA] = countdown;
    if (countdown == 0) {
      g_battle_work[2]++;
    }
  }
}
