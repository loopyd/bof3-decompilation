#include "bof3/bof3.h"

/* Shared battle work pointer cell; the other targets that touch the same cell
 * bind it as the same canonical volatile pointer (see
 * include/bof3/battle/battle15_internal.h). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0330
 * @behavior Seeds the scratchpad work object at pointer cell 0x1F800044 for a
 * new motion: both words of the motion pair at offsets 0x40 and 0x44 are set to
 * 0x10000, work byte 0x48 is set to 2 (the same 0x40/0x44/0x48 triple that
 * 0x801E47A4 and 0x801E3334 write), work byte 9 is seeded with the countdown
 * value 0x1E, and work byte 2 is advanced by one. The original re-reads the
 * pointer cell before each later field (three loads of the cell, each followed
 * by its own stores), because the stores through the loaded pointer may alias
 * the cell; the cell is volatile, so every evaluation is a fresh load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initWorkMotionFieldsBmagicMagic00800_801F0330(void) {
  u8 *work;

  work = g_battle_work;
  *(u32*)(work + 0x40) = 0x10000u;
  *(u32*)(work + 0x44) = 0x10000u;
  work[0x48] = 2;
  work = g_battle_work;
  work[9] = 0x1E;
  work = g_battle_work;
  work[2]++;
}
