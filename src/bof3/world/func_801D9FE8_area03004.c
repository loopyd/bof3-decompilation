#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 handler: while the shared mode byte at 0x80144125 is still
 * 0 and the shared state byte at 0x8014403D holds 1, it stores 4 in byte 9 of
 * the work record published at the scratchpad cursor 0x1F800044, dispatches
 * sound cue 0x102 through func_8015DF18 and advances the record dispatch byte
 * 3; the dim tile is appended through appendDimTile last and unconditionally.
 * @source 0x801D9FE8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D9FE8(void) {
  if (modeByte == 0 && D_8014403D == 1) {
    D_1F800044[9] = 4;
    func_8015DF18(0x102);
    D_1F800044[3]++;
  }
  appendDimTile();
}
