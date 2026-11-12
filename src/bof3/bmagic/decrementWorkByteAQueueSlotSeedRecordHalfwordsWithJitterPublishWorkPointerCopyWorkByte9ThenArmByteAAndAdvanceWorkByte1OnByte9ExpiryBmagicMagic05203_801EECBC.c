#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map as g_battle_work. The original re-materializes and reloads the cell
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) before every access - seven times in this
 * body - so each site below reads this address-derived symbol itself. The
 * struct view only names the three bytes this body touches. */
typedef struct BmagicMagic05203Work {
  u8 unk_00[1];
  u8 unk_01;
  u8 unk_02[7];
  u8 unk_09;
  u8 unk_0A;
} BmagicMagic05203Work;

extern BmagicMagic05203Work *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared record pointer cell at 0x8014638C in main RAM, outside this overlay's
 * image and already bound by this target's generated undefined_syms_auto.txt.
 * The three halfwords this body copies into the claimed slot live at 0x2E, 0x30
 * and 0x32 of the record that cell points at. The original reloads the cell
 * once per statement, three `lui $v0,0x8014 / lw $v0,0x638C($v0)` pairs in all,
 * so the pointer is read through this volatile-qualified declaration: without
 * that qualification the compiler folds the third read into the second and the
 * body lands two instructions short of the original. */
extern u8 *volatile D_8014638C; /* @source 0x8014638C @kind data */

/* Action-slot pool at 0x801EC330 in main RAM, whose 0x78-byte stride these
 * accesses scale by. The original reaches each field as `lui $at,%hi(ADDR)` +
 * `addu $at,$at,index*0x78` + `sb|sh|sw %lo(ADDR)($at)` rather than through one
 * shared record base, so every addressed field keeps its own flat symbol and
 * the generated undefined_syms_auto.txt already binds all seven addresses. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC35E[]; /* @source 0x801EC35E @kind data */
extern u8 D_801EC360[]; /* @source 0x801EC360 @kind data */
extern u8 D_801EC362[]; /* @source 0x801EC362 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable slot allocator at 0x801E590C: it returns the index of
 * the slot it claimed, whose low byte this body scales by 0x78 without
 * comparing it against the 0xFF pool-full answer. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* PsyQ C runtime. The body calls it three times: the result of the first call
 * is discarded, only the low four bits of the second and the low five bits of
 * the third are kept. */
extern int rand(void);

/* @source 0x801EECBC
 * @behavior Countdown step of the overlay work object at scratchpad cell
 *           0x1F800044. Decrements the work object's byte 0xA and, while the
 *           prior value was nonzero, returns having written nothing else. On a
 *           zero prior value it claims one 0x78-byte action slot of the pool at
 *           0x801EC330 through func_801E590C(1, 0x50), truncating the returned
 *           index to a byte and ignoring the pool-full answer; clears the slot's
 *           bytes at 0x801EC331 and 0x801EC332; and publishes a freshly
 *           reloaded work pointer in the slot word at 0x801EC3A4. It then seeds
 *           the slot from the shared record at 0x8014638C: halfword 0x801EC35E
 *           is the record's signed halfword 0x2E plus `rand() & 0xF`, halfword
 *           0x801EC360 is the record's unsigned halfword 0x30 minus
 *           `rand() & 0x1F`, and halfword 0x801EC362 is the record's unsigned
 *           halfword 0x32. The body makes three rand() calls: the first result
 *           is discarded, the second feeds the 0x2E jitter and the third the
 *           0x30 jitter, and the record cell is read once in each statement. One
 *           more work-cell read copies the work object's byte 9 into the slot
 *           byte at 0x801EC339. The body then decrements that same work byte 9:
 *           a nonzero prior value rearms work byte 0xA to 8, while a zero prior
 *           value leaves byte 9 wrapped at 0xFF, rearms byte 0xA to 0x2D and
 *           increments work byte 1, the state byte the overlay's dispatchers
 *           scale. Takes no arguments, returns nothing and keeps a 0x18-byte
 *           frame for $ra and $s0, which holds the scaled slot index across the
 *           four calls.
 * @status partial
 * @match 98.92
 * @residual 92 of 93 instructions match and the image is the original 372
 *           bytes. The one difference is the record halfword 0x2E load: the
 *           original sign extends it (lh) and this profile emits lhu for every
 *           spelling tried, listed in the lane ledger - signed record field,
 *           s16 temporary, signed destination, volatile field, s16-typed cell,
 *           and swapped or cast addends. The three cell reloads at 0x8014638C
 *           are reproduced by the volatile qualification on that cell; the
 *           work pointer read is carried in a local so the byte-9 load stays
 *           after the 0x801EC362 store, as the original schedules it.
 */
void decrementWorkByteAQueueSlotSeedRecordHalfwordsWithJitterPublishWorkPointerCopyWorkByte9ThenArmByteAAndAdvanceWorkByte1OnByte9ExpiryBmagicMagic05203_801EECBC(
    void) {
  u8 *work;
  u32 slot;

  if (g_battle_work->unk_0A-- == 0) {
    slot = (u8)func_801E590C(1, 0x50) * 0x78;
    *(u8 **)&D_801EC3A4[slot] = (u8 *)g_battle_work;
    D_801EC331[slot] = 0;
    D_801EC332[slot] = 0;
    rand();
    *(u16 *)&D_801EC35E[slot] = *(s16 *)(D_8014638C + 0x2E) + (rand() & 0xF);
    *(u16 *)&D_801EC360[slot] = *(u16 *)(D_8014638C + 0x30) - (rand() & 0x1F);
    /* The work pointer for the byte-9 copy is read before the halfword store
     * so the byte load itself stays after it, as the original schedules it. */
    work = (u8 *)g_battle_work;
    *(u16 *)&D_801EC362[slot] = *(u16 *)(D_8014638C + 0x32);
    D_801EC339[slot] = work[9];
    if (g_battle_work->unk_09-- != 0) {
      g_battle_work->unk_0A = 8;
    } else {
      g_battle_work->unk_0A = 0x2D;
      g_battle_work->unk_01++;
    }
  }
}
