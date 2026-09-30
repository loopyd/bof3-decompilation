#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7DC8
 * @behavior Advances the scratch work object's entry-effect handler: re-reads
 * the published work-object pointer cell at 0x1F800044 and emits five
 * func_801F85A8 draws, the first from that object's word at offset 0x18 with
 * (0x200, 0, 0) and the second from the word at offset 0x1C with (0x71, 0,
 * 0x8E3), then three byte draws, from the object's bytes at offsets 0x0B, 0x08
 * and 0x0A with (0x155, 0, 0x200), (0xE3, 0, 0xE38) and (0x31C, 0, 0x555);
 * after that it adds 8 to the byte at offset 0x0A and, once that byte reads
 * exactly 0x10 or exactly 0x60, queues frontend cue 0x20E, the second of the
 * two branches also setting the object's dispatch byte at offset 0x01 to 6.
 * Takes no arguments and returns nothing; the address is entry 16 (word at
 * 0x801FC9C0) of the per-frame handler table at 0x801FC980 indexed by byte 0x01
 * of that work object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7DC8(void) {
  func_801F85A8(*(s32 *)(D_1F800044 + 0x18), 0x200, 0, 0);
  func_801F85A8(*(s32 *)(D_1F800044 + 0x1C), 0x71, 0, 0x8E3);
  func_801F85A8(D_1F800044[0x0B], 0x155, 0, 0x200);
  func_801F85A8(D_1F800044[0x08], 0xE3, 0, 0xE38);
  func_801F85A8(D_1F800044[0x0A], 0x31C, 0, 0x555);
  D_1F800044[0x0A] += 8;
  if (D_1F800044[0x0A] == 0x10) {
    game_queue_frontend_cue(0x20E);
  }
  if (D_1F800044[0x0A] == 0x60) {
    game_queue_frontend_cue(0x20E);
    D_1F800044[1] = 6;
  }
}
