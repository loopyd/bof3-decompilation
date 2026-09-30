#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7B0C
 * @behavior Runs one per-frame step of the scratchpad work object published at
 * 0x1F800044: it passes the object's word at offset 0x18 and the three fixed
 * words 0x200/0/0 to func_801F85A8, then re-reads the object pointer, advances
 * that word at offset 0x18 by 8, and once the word reads exactly 0x60 queues
 * the frontend cue 0x20E and moves the object's dispatch byte at offset 0x01 to
 * 2. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7B0C(void) {
  s32 value;

  func_801F85A8(*(s32 *)(D_1F800044 + 0x18), 0x200, 0, 0);

  value = *(s32 *)(D_1F800044 + 0x18) + 8;
  *(s32 *)(D_1F800044 + 0x18) = value;

  if (value == 0x60) {
    game_queue_frontend_cue(0x20E);
    D_1F800044[1] = 2;
  }
}
