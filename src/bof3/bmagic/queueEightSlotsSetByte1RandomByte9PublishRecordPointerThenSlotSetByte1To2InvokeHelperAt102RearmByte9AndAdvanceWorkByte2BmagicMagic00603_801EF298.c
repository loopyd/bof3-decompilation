#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original reads the cell once before the frame is
 * created, for the byte-0x0B gate, and re-reads it with its own lui / lw pair at
 * 0x7B0 and 0x7C0 for the two closing stores, so each of those two sites keeps
 * its own plain cell read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer cell at 0x801EB4E0 in main RAM, outside this
 * overlay image and already bound by the generated undefined_syms_auto.txt. The
 * word held there is published into the claimed action slot at +0x74 and the
 * record's byte 0x0B is the counter this body raises once per claimed slot; the
 * cell is re-read for every such access, so each site keeps its own local and
 * the byte view below is the only view used. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: bytes +0x01,
 * +0x09 and +0x0B and the pointer word at +0x74 of one entry. The +0x0B byte is
 * the action state that func_800AAEBC also writes. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable helpers, all outside every payload lifted here. The
 * slot allocator at 0x801E590C returns the index of the entry it claimed; this
 * body ignores the 0xFF "pool full" answer the sibling lift of the same family
 * tests, so no comparison against it appears in the original. func_8015D71C
 * takes one literal from this body and returns nothing, and func_801E5988 takes
 * no arguments. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_801E5988(void); /* @source 0x801E5988 */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* PsyQ C runtime. Only the low four bits of the result are used, as the
 * one-based byte written into the slot field at +0x09. */
extern int rand(void);

/* @source 0x801EF298
 * @behavior Entry index 4 of the thirteen-word handler run
 *           0x801EEC04..0x801EEC34, this target's tabledata segment; the word at
 *           0x801EEC14 holds this address and is the only word in the payload
 *           that does, so the entry is reached through that run and has no jal
 *           caller. Reads the scratchpad work-object pointer cell at 0x1F800044
 *           once before the 0x20-byte frame is created and branches on the work
 *           object's byte 0x0B. While that byte is zero the body claims eight
 *           action slots from the pool at 0x801EC330 through
 *           func_801E590C(1, 0x53), scales each returned index by the 0x78-byte
 *           slot stride and stamps four fields of the entry: byte +0x01 becomes
 *           1, byte +0x0B receives the loop ordinal, the pointer word at +0x74
 *           receives the shared battle record pointer read from 0x801EB4E0 and
 *           byte +0x09 receives one plus the low four bits of rand(). Each
 *           iteration then re-reads 0x801EB4E0 and increments that record's byte
 *           0x0B, and the loop repeats while the ordinal stays below eight. After
 *           the eight slots it claims one further slot with the same two
 *           literals and stamps byte +0x01 with 2, publishes the same record
 *           pointer word and increments the record's byte 0x0B a ninth time, then
 *           calls the shared helper func_8015D71C with 0x102. Finally it
 *           re-reads the cell twice more, setting work byte 0x09 to 0x10 and
 *           incrementing work byte 0x02, which advances this overlay's work byte
 *           2 sequence. When work byte 0x0B was non-zero instead, the body reads
 *           0x801EB4E0, decrements that record's byte 0x0B by one at byte width
 *           and invokes the shared helper func_801E5988. Takes no arguments,
 *           returns nothing, and the 0x20-byte frame keeps $ra at 0x1C and
 *           $s0..$s2 at 0x10..0x18 across the calls. The roles of the slot bytes
 *           and of the record byte 0x0B beyond the action-slot pool role that
 *           func_800AAEBC documents remain unestablished, so the name stays
 *           offset-anchored. The closing slot stamp keeps its own offset and
 *           record locals because the original leaves that block's scaled index
 *           in $v1 and reads the record cell a second time into $a1 before the
 *           pointer word store, which is what fills that load's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueEightSlotsSetByte1RandomByte9PublishRecordPointerThenSlotSetByte1To2InvokeHelperAt102RearmByte9AndAdvanceWorkByte2BmagicMagic00603_801EF298(void) {
  u8 *work;
  u8 *record;
  u8 *recordCount;
  u8 *record2;
  u8 *recordCount2;
  u32 offset;
  u32 offset2;
  u8 i;

  work = g_battle_work;
  if (work[0x0B] == 0) {
    i = 0;
    do {
      offset = (u8)func_801E590C(1u, 0x53u) * 0x78;
      record = D_801EB4E0;
      D_801EC331[offset] = 1;
      D_801EC33B[offset] = i;
      *(u32 *)&D_801EC3A4[offset] = (u32)record;
      i++;
      D_801EC339[offset] = (rand() & 0xF) + 1;
      recordCount = D_801EB4E0;
      recordCount[0x0B]++;
    } while (i < 8);
    offset2 = (u8)func_801E590C(1u, 0x53u) * 0x78;
    record2 = D_801EB4E0;
    D_801EC331[offset2] = 2;
    recordCount2 = D_801EB4E0;
    *(u32 *)&D_801EC3A4[offset2] = (u32)record2;
    recordCount2[0x0B]++;
    func_8015D71C(0x102);
    g_battle_work[9] = 0x10;
    g_battle_work[2]++;
  } else {
    D_801EB4E0[0x0B]--;
    func_801E5988();
  }
}
