#include "bof3/bof3.h"

/* Shared published work-pointer cell: retain each original handoff and reload. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 workRecordAt80146920World03Area13413[]; /* @source 0x80146920 @kind unknown */
extern s32 D_80146954;
extern s32 D_80145EC4;
extern u8 D_80145E90[];
extern u8* D_80146250;

/* @source 0x801F39C0
 * @behavior Saves the scratchpad work pointer, publishes the work record at
 * 0x80146920, compares its signed word at offset 0x34 (0x80146954) against the
 * signed word at 0x80145EC4, and publishes 0x80145E90 at 0x80146250. Restores
 * the saved scratchpad pointer, briefly publishes 0x80145E90 and restores it
 * again. Selects state byte 1 = 10 when the first word is smaller; otherwise
 * selects state 13 and clears byte 9 through the reloaded scratchpad work
 * pointer. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectScratchState10Or13ByWorkWordWorld03Area13413_801F39C0(void) {
  u8* saved;
  s32 smaller;

  saved = D_1F800044;
  D_1F800044 = workRecordAt80146920World03Area13413;
  smaller = D_80146954 < D_80145EC4;
  D_80146250 = D_80145E90;
  D_1F800044 = saved;
  D_1F800044 = D_80145E90;
  D_1F800044 = saved;
  if (!smaller) {
    saved[1] = 13;
    D_1F800044[9] = 0;
  } else {
    saved[1] = 10;
  }
}
