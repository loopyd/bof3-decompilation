#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8014
 * @behavior Advances the scratch work object's entry-effect handler: re-reads
 * the published work-object pointer cell at 0x1F800044 and emits seven
 * func_801F85A8 draws -- the object's word at 0x18 with (0x200, 0, 0), its
 * word at 0x1C with (0x71, 0, 0x8E3), and its bytes at 0x0B, 0x08, 0x0A, 0x06
 * and 0x07 with (0x155, 0, 0x200), (0xE3, 0, 0xE38), (0x31C, 0, 0x555),
 * (0x238, 0, 0xAAA) and (0, 0, 0x71C). It then adds 8 to the byte at offset
 * 0x07 and, once that byte reads exactly 0x60, queues frontend cue 0x20E and
 * sets the object's dispatch byte at offset 0x01 to 8. Takes no arguments and
 * returns nothing; the address is the per-frame handler table entry selected by
 * byte 0x01 of that work object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F8014(void) {
  func_801F85A8(*(s32 *)(D_1F800044 + 0x18), 0x200, 0, 0);
  func_801F85A8(*(s32 *)(D_1F800044 + 0x1C), 0x71, 0, 0x8E3);
  func_801F85A8(D_1F800044[0x0B], 0x155, 0, 0x200);
  func_801F85A8(D_1F800044[0x08], 0xE3, 0, 0xE38);
  func_801F85A8(D_1F800044[0x0A], 0x31C, 0, 0x555);
  func_801F85A8(D_1F800044[0x06], 0x238, 0, 0xAAA);
  func_801F85A8(D_1F800044[0x07], 0, 0, 0x71C);
  D_1F800044[0x07] += 8;
  if (D_1F800044[0x07] == 0x60) {
    game_queue_frontend_cue(0x20E);
    D_1F800044[1] = 8;
  }
}
