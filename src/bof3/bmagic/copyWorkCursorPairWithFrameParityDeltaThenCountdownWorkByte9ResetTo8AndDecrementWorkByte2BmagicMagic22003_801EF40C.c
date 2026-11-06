#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address (config/targets/shared/symbols.txt) and the sibling
 * frame-parity lift advanceWorkCursorOnFrameParityBossBoss01816_800C203C reads
 * the same word, its low bit as a per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared battle record pointer in main RAM. This target's generated
 * undefined_syms_auto.txt already binds 0x801EB4E0 for the overlay's other
 * handlers (func_801EF078, func_801EF4EC, func_801EF294), so no target-map row
 * is needed; the sibling lift incrementWorkByte2WhenGlobalWorkByteBBelow3
 * BmagicMagic22003_801EF888 reads the same record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. The original re-materializes the cell
 * with its own lui/lw pair at each of the five reads, so the cell is named
 * rather than written as a bare scratchpad constant: a constant address folds
 * into one CSE'd register plus lw 0(reg) per access, while the named symbol
 * keeps one lui 0x1F80 plus lw 0x44(reg) per read. The cell is not volatile,
 * because the stores through the loaded pointer already separate the reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF40C
 * @behavior Per-frame step of the overlay's work-object handler selected by
 * work-object byte 0x1 = 4: it is word 4 of the thirteen-word handler run that
 * the overlay's dispatcher 0x801EEF78 indexes at 0x801F06DC, so work-object
 * byte 0x1 holds 4 while this body runs. The scratchpad work-object pointer
 * cell at 0x1F800044 names the work object and the shared battle record pointer
 * at 0x801EB4E0 names the record whose cursor pair this handler projects onto
 * that object. Bit 0 of the shared frame counter at 0x80143E6C picks the sign
 * of the offset word 0xC the work object carries: on a set bit the object's
 * cursor word 0x34 becomes the record's word 0x34 plus that offset and its
 * cursor word 0x38 becomes the record's word 0x38 minus it, and on a clear bit
 * both terms invert, so the pair swings through the record's cursor position
 * along one diagonal and back on the next frame. Each arm reads the record
 * pointer and the pointer cell once and the offset word once per cursor word,
 * with no local shared between the pairs. The handler then counts the object's
 * countdown byte 0x9 down by one through a single pointer-cell read
 * (lbu/addiu/sb) and, when the truncation to a byte is zero, re-reads the cell
 * to store 0x8 back into byte 0x9 and re-reads it once more to decrement the
 * work-object phase byte 0x2, which steps the object one phase back. The
 * function is a leaf with no frame and no saved registers; it takes no
 * arguments, returns nothing and calls nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkCursorPairWithFrameParityDeltaThenCountdownWorkByte9ResetTo8AndDecrementWorkByte2BmagicMagic22003_801EF40C(void) {
  u8 *record;
  u8 *work;
  u8 *countdown;
  u8 count;

  if (D_80143E6C & 1u) {
    record = D_801EB4E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) + *(s32 *)(work + 0xC);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) - *(s32 *)(work + 0xC);
  } else {
    record = D_801EB4E0;
    work = g_battle_work;
    *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) - *(s32 *)(work + 0xC);
    *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) + *(s32 *)(work + 0xC);
  }

  countdown = g_battle_work;
  count = (u8)(countdown[9] - 1);
  countdown[9] = count;
  if (count == 0) {
    g_battle_work[9] = 8;
    g_battle_work[2]--;
  }
}
