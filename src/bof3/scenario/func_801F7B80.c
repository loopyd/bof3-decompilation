#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7B80
 * @behavior Advances the scratch work object's entry-effect handler: re-reads
 * the published work-object pointer cell at 0x1F800044 and emits two
 * func_801F85A8 draws, the first from that object's word at offset 0x18 with
 * (0x200, 0, 0) and the second from the word at offset 0x1C with (0x71, 0,
 * 0x8E3); then adds 0x10 to the word at offset 0x1C and queues frontend cue
 * 0x20E when it reads 0x20, and queues the same cue and sets the object's
 * dispatch byte at offset 0x01 to 3 when it reads 0xA0. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7B80(void) {
  func_801F85A8(*(s32 *)(D_1F800044 + 0x18), 0x200, 0, 0);
  func_801F85A8(*(s32 *)(D_1F800044 + 0x1C), 0x71, 0, 0x8E3);
  if ((*(s32 *)(D_1F800044 + 0x1C) += 0x10) == 0x20) {
    game_queue_frontend_cue(0x20E);
  }
  if (*(s32 *)(D_1F800044 + 0x1C) == 0xA0) {
    game_queue_frontend_cue(0x20E);
    D_1F800044[1] = 3;
  }
}
