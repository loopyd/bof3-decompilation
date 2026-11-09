#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * and shared with the exact siblings of this overlay. The original
 * re-materialises the cell with its own lui / lw pair before each of the four
 * reads this body makes - 0x730, 0x754, 0x7CC and 0x84C - and every reload sits
 * behind a store issued through the pointer loaded before it, so each read is
 * written at its own site. Folding two sites onto one loaded base kept the 344
 * byte length but re-assigned the pointer registers (measured 64/86 and 73/86
 * matching instructions), while the per-site reads measure 86/86. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared per-frame counter at 0x80143E6C in main RAM, already bound by the
 * shared symbol map, so no target-map row is needed. Only its low two bits are
 * read here: the every-fourth-frame gate the sibling spawners of this family
 * also use. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Shared battle record pointer at 0x801EB4E0, outside this overlay image and
 * already bound by the generated undefined_syms_auto.txt. The word stored there
 * is published into the queued slot at +0x74 and its byte 0x0B is the count
 * this body increments; the cell is read twice, once for the publish (0x7B4)
 * and once before that increment (0x804), so each read keeps its own local. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: byte +0x01
 * and the words at +0x34, +0x38, +0x3C and +0x74 of one entry. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable helpers, both outside every payload lifted here. The
 * first takes no arguments and returns nothing; the slot allocator at
 * 0x801E590C returns the index of the entry it claimed, or 0xFF when the pool
 * holds no free entry. */
void func_800AF764(void); /* @source 0x800AF764 */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Overlay routine that follows this body in the payload and stays an asm
 * boundary of this target, so it keeps its raw name and needs no map row. */
void func_801F02A0(void); /* @source 0x801F02A0 */

/* @source 0x801EF320
 * @behavior Entry 2 of the three-entry handler table at 0x801F0A1C that this
 *           overlay's work-byte-2 dispatcher
 *           dispatchWorkByte2HandlerThenDrawByWorkByte4BmagicMagic04403_801EEE44
 *           indexes with the scratchpad work object's byte 0x2, so no jal in
 *           this payload reaches this address; the table words
 *           0x801EEEFC/0x801EF138/0x801EF320 are the three state blocks that
 *           follow that dispatcher. The cell at 0x1F800044 is read once before
 *           the 0x18-byte frame is created. While the work object's byte 0x0A
 *           is below 0x10 the body counts up: it adds one to work byte 0x0B and
 *           one to work byte 0x0A through a second read of the cell, then calls
 *           the shared helper func_800AF764 with the byte-0x0A store filling
 *           that call delay slot and calls the overlay routine func_801F02A0.
 *           On the frames where the low two bits of the shared counter at
 *           0x80143E6C are clear - every fourth frame - it claims an action slot
 *           through func_801E590C(1, 6) and, unless that allocator reports 0xFF,
 *           scales the returned index by the 0x78-byte slot stride and fills
 *           four fields of the entry: byte +0x01 becomes 1, the pointer word at
 *           +0x74 receives the shared battle record pointer read from
 *           0x801EB4E0, and the work object words 0x34, 0x38 and 0x3C are copied
 *           into the entry words at the same offsets through the third read of
 *           the cell. It then re-reads 0x801EB4E0 and increments that record
 *           byte 0x0B. Outside the tick gate it re-reads the cell and, while
 *           work byte 0x0A is exactly 0x10, copies that byte to work byte 0x09
 *           and increments the work object state byte 0x02 through a last read
 *           of the cell, which advances this overlay's work byte 2 sequence.
 *           Every other byte-0x0A value branches straight to the epilogue. Takes
 *           no arguments, returns nothing, and the 0x18-byte frame keeps only
 *           $ra at 0x10 because of the three calls. The roles of work bytes 9,
 *           0x0A and 0x0B, of the shared record byte 0x0B and of the slot field
 *           triplet remain unestablished, so the name stays offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSlotSetByte1PublishRecordPointerCopyWorkWordTripletEveryFourthTickWhileByteABelow16ThenAdvanceWorkByte2At16BmagicMagic04403_801EF320(void) {
  u8 *work;
  u8 *record;
  u8 *workAdvance;
  u8 *recordCount;
  u8 *slotWork;
  u32 slot;
  u32 offset;
  u8 count;

  work = g_battle_work;
  if (work[0xA] < 0x10) {
    work[0xB]++;
    workAdvance = g_battle_work;
    workAdvance[0xA]++;
    func_800AF764();
    func_801F02A0();
  }
  if ((D_80143E6C & 3) == 0) {
    slot = (u8)func_801E590C(1u, 6u);
    if (slot != 0xFF) {
      offset = slot * 0x78;
      record = D_801EB4E0;
      D_801EC331[offset] = 1;
      slotWork = g_battle_work;
      *(u32 *)&D_801EC3A4[offset] = (u32)record;
      *(u32 *)&D_801EC364[offset] = *(u32 *)(slotWork + 0x34);
      *(u32 *)&D_801EC368[offset] = *(u32 *)(slotWork + 0x38);
      recordCount = D_801EB4E0;
      *(u32 *)&D_801EC36C[offset] = *(u32 *)(slotWork + 0x3C);
      recordCount[0xB]++;
    }
  }
  count = g_battle_work[0xA];
  if (count == 0x10) {
    g_battle_work[9] = count;
    g_battle_work[2]++;
  }
}
