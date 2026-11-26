#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEC70
 * @behavior Overlay step handler: copies record byte 8 and the record word
 * triplet at 0x34, 0x38 and 0x3C, read through the shared record pointer at
 * 0x801EB4E0, into the scratchpad work object published at the pointer cell
 * 0x1F800044, clears work byte 0xB and increments work byte 1. The record and
 * work cells are re-read after the byte copy because a store through the loaded
 * work pointer may alias the cell; the third record word is held in a local so
 * its load precedes the byte 0xB clear and its store follows it, and the work
 * cell is re-read once more before the byte 1 increment. Leaf with no frame;
 * table word 0x801EEC04 selects this step through work byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordByte8Word(void) {
  u8 *record;
  u8 *work;
  u8 *step;
  u32 value;

  g_battle_work[8] = D_801EB4E0[8];
  record = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  value = *(u32 *)(record + 0x3C);
  work[0xB] = 0;
  step = g_battle_work;
  *(u32 *)(work + 0x3C) = value;
  step[1]++;
}
