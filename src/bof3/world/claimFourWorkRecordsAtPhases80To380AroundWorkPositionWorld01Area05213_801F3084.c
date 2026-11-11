#include "bof3/bof3.h"

/* @source 0x801F3158 */
extern u8 *func_801F3158(void);
/* @source 0x801782FC */
extern s32 func_801782FC(u16 phase);
/* @source 0x801783C8 */
extern s32 func_801783C8(u16 phase);

/* Scratch work object the shared phase samplers scale and this overlay reads as
 * three position words. The cell is outside this payload, and this target's map
 * binds no name for it, so it keeps the raw address spelling. The original
 * re-materialises the cell with its own lui/lw pair at each use because the
 * intervening calls may republish it, which is the named-cell form rather than
 * the inline constant one. */
extern s32 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F3084
 * @behavior Claims four work records, one per phase step of 0x100 from 0x80.
 *           Each pass asks func_801F3158 for the first record of the
 *           0x800E4800 work table whose occupancy byte reads zero and skips the
 *           rest of the pass when it returns none. For a claimed record it sets
 *           occupancy byte 0x15 to 1 and byte 0x16 to 8, then seeds the three
 *           position words from the scratch work object published at cell
 *           0x1F800044: word 0x00 takes that object's word 0x34 plus the table
 *           offset func_801783C8 yields for the phase, word 0x04 takes its word
 *           0x38 plus the offset func_801782FC yields for the same phase, and
 *           word 0x08 copies its word 0x3C. Word 0x10 becomes 0x1800 and byte
 *           0x14 becomes 0xC0. The phase and the byte counter advance in a
 *           bottom-tested loop that runs while the counter is below 4, and the
 *           cell is re-read after each sampler call because a call may
 *           republish it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFourWorkRecordsAtPhases80To380AroundWorkPositionWorld01Area05213_801F3084(void) {
  u8 *record;
  s32 phase;
  u8 i;

  phase = 0x80;
  i = 0u;

  do {
    record = func_801F3158();
    if (record != 0) {
      record[0x15] = 1;
      record[0x16] = 8;
      *(s32 *)(record + 0x00) = D_1F800044[0xD] + func_801783C8(phase) * 16;
      *(s32 *)(record + 0x04) = D_1F800044[0xE] + func_801782FC(phase) * 16;
      *(s32 *)(record + 0x08) = D_1F800044[0xF];
      *(s32 *)(record + 0x10) = 0x1800;
      record[0x14] = 0xC0;
    }
    i += 1u;
    phase += 0x100;
  } while (i < 4u);
}
