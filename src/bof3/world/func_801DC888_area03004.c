#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 mode gate: while the shared mode byte modeByte holds zero,
 * requests the frontend selection effect through func_8015D4F8 with
 * (0, 0, 0x14, 0x10), dispatches sound cue 0x102 through func_8015DF18,
 * increments the shared state byte 0x80144281, clears byte 6 of the work
 * record published at the scratchpad cursor 0x1F800044 and advances that
 * record's dispatch byte 3.
 * @source 0x801DC888
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DC888(void) {
  if (modeByte == 0) {
    func_8015D4F8(0, 0, 0x14, 0x10);
    func_8015DF18(0x102);
    D_80144281++;
    D_1F800044[6] = 0;
    D_1F800044[3]++;
  }
}
