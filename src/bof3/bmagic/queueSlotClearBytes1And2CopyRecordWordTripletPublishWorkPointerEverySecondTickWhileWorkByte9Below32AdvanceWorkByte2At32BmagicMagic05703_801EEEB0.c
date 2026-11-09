#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, shared with the exact
 * siblings of this overlay. The original re-materialises the cell with its own
 * lui 0x1F80 / lw 0x44 pair at both reads this body makes - one before the
 * work-byte-9 compare and one after the record triplet is stored - so each read
 * is written at its own site; the second read also sits behind stores issued
 * through the pointer loaded first, so it cannot be folded onto it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared per-frame counter at 0x80143E6C in main RAM, already bound by the
 * shared symbol map, so no target-map row is needed. Only bit 0 is read here:
 * the every-second-frame gate the sibling spawners of this family also use. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Shared battle record pointer at 0x801EB4E0, outside this overlay image and
 * already bound by the generated undefined_syms_auto.txt. The one load of the
 * cell serves all three reads: its words 0x34 and 0x38 are copied into the slot
 * triplet and its word 0x3C into the slot word at the same offset. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: bytes +0x01
 * and +0x02 and the words at +0x34, +0x38, +0x3C and +0x74 of one entry. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared slot allocator of the main executable: returns the index of the entry
 * it claimed, or 0xFF when the pool holds no free entry. 0x41 is the slot class
 * this family claims; the sibling spawner in bmagic passes 6. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Shape read off the original bytes. The record word 0x3C is carried in the
 * record variable itself rather than in a fresh local: the original loads it as
 * `lw $a0,0x3C($a0)`, overwriting the base register that is dead after that
 * instruction, and stores it from the same register. Spelling the third store as
 * a direct `*(u32 *)&D_801EC36C[offset] = *(u32 *)(record + 0x3C)` instead lets
 * the load take its own register, which then collides with the reloaded work
 * pointer: the record base moved to $a1, the work pointer to $a0 and the third
 * word to $v1, measuring 50/61 with the first difference at +0x70. Writing the
 * word back into the record variable removes that extra live value and the
 * original assignment returns: record $a0, work $v1, third word in $a0,
 * 61/61. Reading the word before the work reload in the source still emits the
 * reload first (0x34C) and the word read second (0x354) at 61/61; placing the
 * read after the +0x74 store measures 60/61 with the first difference at +0xa4,
 * so the statement order below is the one the original scheduler reproduces. */

/* @source 0x801EEEB0
 * @behavior Slot spawner of this overlay's work handler sequence. The span is
 *           reached only from the pointer run in this target's table data at
 *           0x801EEC18, so no jal in the image names it. While bit 0 of the
 *           shared per-frame counter at 0x80143E6C is set - every second frame
 *           - it reads the scratchpad work object through the pointer cell at
 *           0x1F800044 and, while work byte 0x09 is below 0x20, claims an action
 *           slot with func_801E590C(1, 0x41) and, unless the allocator reports
 *           0xFF, scales the returned index by the 0x78-byte slot stride and
 *           fills one entry: bytes +0x01 and +0x02 are cleared, the shared
 *           record pointer at 0x801EB4E0 is read once and its words 0x34, 0x38
 *           and 0x3C are copied into the entry words at the same offsets, and
 *           the entry word at +0x74 receives a second read of the work-object
 *           pointer cell. It then increments work byte 0x09 through that second
 *           work pointer, which is the count that caps this spawner. On the
 *           frames where work byte 0x09 has already reached 0x20 it leaves the
 *           slot pool alone and increments work byte 0x02 instead, the index the
 *           state-2 handler at 0x801EF83C dispatches the second-phase table
 *           with. The roles of work bytes 0x02 and 0x09, of slot bytes 0x01 and
 *           0x02 and of the slot word at 0x74 stay unestablished, so they remain
 *           offset-anchored. Takes no arguments, returns nothing through the
 *           jr $ra at 0x801EEF9C inside a 0x18-byte frame that keeps $ra at
 *           0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSlotClearBytes1And2CopyRecordWordTripletPublishWorkPointerEverySecondTickWhileWorkByte9Below32AdvanceWorkByte2At32BmagicMagic05703_801EEEB0(void) {
  u8 *work;
  u8 *record;
  u8 *slotWork;
  u32 slot;
  u32 offset;

  if ((D_80143E6C & 1) != 0) {
    work = g_battle_work;
    if (work[9] < 0x20) {
      slot = (u8)func_801E590C(1u, 0x41u);
      if (slot != 0xFF) {
        offset = slot * 0x78;
        D_801EC331[offset] = 0;
        D_801EC332[offset] = 0;
        record = D_801EB4E0;
        *(u32 *)&D_801EC364[offset] = *(u32 *)(record + 0x34);
        *(u32 *)&D_801EC368[offset] = *(u32 *)(record + 0x38);
        record = (u8 *)(*(u32 *)(record + 0x3C));
        slotWork = g_battle_work;
        *(u32 *)&D_801EC3A4[offset] = (u32)slotWork;
        *(u32 *)&D_801EC36C[offset] = (u32)record;
        slotWork[9]++;
      }
    } else {
      work[2]++;
    }
  }
}
