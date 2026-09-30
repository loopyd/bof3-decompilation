#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7C34
 * @behavior Advances the scratch work object's entry-effect handler: re-reads
 * the published work-object pointer cell at 0x1F800044 and emits two
 * func_801F85A8 draws, the first from that object's word at offset 0x18 with
 * (0x200, 0, 0) and the second from the word at offset 0x1C with (0x71, 0,
 * 0x8E3), then a third draw from the object's byte at offset 0x0B with
 * (0x155, 0, 0x200); it then adds 0x10 to that byte at offset 0x0B and, once
 * the byte reads 0x60, queues frontend cue 0x20E and sets the object's
 * dispatch byte at offset 0x01 to 4. Takes no arguments and returns nothing;
 * the address is entry 14 of the per-frame handler table at 0x801FC980 indexed
 * by byte 0x01 of that work object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7C34(void) {
  func_801F85A8(*(s32 *)(D_1F800044 + 0x18), 0x200, 0, 0);
  func_801F85A8(*(s32 *)(D_1F800044 + 0x1C), 0x71, 0, 0x8E3);
  func_801F85A8(D_1F800044[0x0B], 0x155, 0, 0x200);
  D_1F800044[0x0B] += 0x10;
  if (D_1F800044[0x0B] == 0x60) {
    game_queue_frontend_cue(0x20E);
    D_1F800044[1] = 4;
  }
}
