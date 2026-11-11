#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map as g_battle_work. The original re-materializes and reloads the cell
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) before every access, so each site below
 * keeps its own read of this address-derived symbol. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared record pointer cell at 0x8014638C in main RAM, outside this overlay's
 * image and already bound by this target's generated undefined_syms_auto.txt.
 * The word held there is the record whose halfwords 0x2E/0x30 and words
 * 0x34/0x38/0x3C this body copies into the work object and whose byte 0x8 it
 * stamps into every claimed slot. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind data */

/* Action-slot pool at 0x801EC330 in main RAM, whose 0x78-byte stride these
 * accesses scale by. The original reaches each field as `lui $at,%hi(ADDR)` +
 * `addu $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one
 * shared record base, so every addressed field keeps its own flat symbol and
 * the generated undefined_syms_auto.txt already binds all eight addresses. */
extern u8 D_801EC338[]; /* @source 0x801EC338 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33A[]; /* @source 0x801EC33A @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Jitter table in this overlay's own image at 0x801F065C, 0x38 bytes long and
 * sitting directly after the two-word handler-pointer table at 0x801F0654.
 * This body writes it; the handler spans at 0x801EF1FC, 0x801EF804 and
 * 0x801F012C reach the same base, and 0x801EF1FC adds a byte index to it
 * (`addu $s0,$t0,$s1` then `addu $s0,$s0,$t1`), so it is a byte table. Its base
 * falls inside the trailing textbin block named D_801F062C, which auto-binds
 * this address through the generated undefined-symbol table. */
extern u8 D_801F065C[]; /* @source 0x801F065C @kind table */

/* Shared main-executable helpers, all outside every payload lifted here. The
 * allocator at 0x801E590C returns the index of the slot it claimed, whose low
 * byte this body scales by 0x78 without comparing it against the 0xFF
 * pool-full answer. func_8015D71C takes one literal and returns nothing. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* PsyQ C runtime. Only the low bit of the first result and the low four bits
 * of the second select the sign and magnitude of each table byte. */
extern int rand(void);

/* @source 0x801EEC9C
 * @behavior Stage 0 of this overlay's five-word handler table at 0x801EEC04,
 *           which func_801EEC18 copies into its frame and dispatches on the
 *           work object's byte 0x1. Reads the shared record pointer cell at
 *           0x8014638C once, then reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once and copies that record's halfwords 0x2E
 *           and 0x30 and words 0x34, 0x38 and 0x3C into the same offsets of
 *           the work object. Fills the 0x38-byte jitter table at 0x801F065C
 *           byte by byte up to its end pointer: each byte takes `rand() & 0xF`
 *           when the low bit of a first rand() is clear and the negation of
 *           that value otherwise. Then claims twelve 0x78-byte action slots of
 *           the pool at 0x801EC330 through func_801E590C(1, 0x17), truncating
 *           the returned index to a byte and ignoring the pool-full answer.
 *           Each claimed slot receives the reloaded work pointer in its word
 *           0x74, the record's byte 0x8 at 0x801EC338, the separately reloaded
 *           work object's words 0x34, 0x38 and 0x3C at 0x801EC364/368/36C, a
 *           byte at 0x801EC339 that starts at 0x30 and drops by four per slot,
 *           the loop ordinal at 0x801EC33B and a cleared byte at 0x801EC33A.
 *           Calls func_8015D71C(0x100) once, then clears the work object's
 *           bytes 9 and 0xA through two separate cell reads and increments its
 *           byte 0x1 through a third, the counter the dispatcher scales by
 *           four, which advances the sequence to stage 1. Takes no arguments,
 *           returns nothing and keeps a 0x20-byte frame for $ra and $s0-$s2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordFieldsSeedRandomTableQueueTwelveSlotsWithDescendingByte9ClearByteAAndAdvanceWorkByte1BmagicMagic09503_801EEC9C(
    void) {
  u8 *record;
  u8 *work;
  u8 *slotWork;
  u8 *table;
  u8 *tableEnd;
  u32 i;
  u8 shade;
  u32 slot;
  u32 value;

  record = D_8014638C;
  work = g_battle_work;
  *(u16 *)(work + 0x2E) = *(u16 *)(record + 0x2E);
  *(u16 *)(work + 0x30) = *(u16 *)(record + 0x30);
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C);
  table = D_801F065C;
  tableEnd = table + 0x38;
  do {
    if (rand() & 1) {
      *table = -(rand() & 0xF);
    } else {
      *table = rand() & 0xF;
    }
    table++;
  } while (table < tableEnd);
  i = 0;
  shade = 0x30;
  for (i = 0; i < 0xC; i++) {
    slot = (u8)func_801E590C(1, 0x17) * 0x78;
    *(u8 **)&D_801EC3A4[slot] = g_battle_work;
    D_801EC338[slot] = record[8];
    slotWork = g_battle_work;
    *(u32 *)&D_801EC364[slot] = *(u32 *)(slotWork + 0x34);
    *(u32 *)&D_801EC368[slot] = *(u32 *)(slotWork + 0x38);
    value = *(u32 *)(slotWork + 0x3C);
    D_801EC339[slot] = shade;
    D_801EC33B[slot] = i;
    D_801EC33A[slot] = 0;
    *(u32 *)&D_801EC36C[slot] = value;
    shade -= 4;
  }
  func_8015D71C(0x100);
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0;
  g_battle_work[1]++;
}
