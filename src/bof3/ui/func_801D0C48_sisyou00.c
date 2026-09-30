#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D0C48
 * @behavior queues frontend cue 0x102 through the EXE-side dispatcher, writes
 *           the phase byte at 0x801D4289 as 4, then advances the handler index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D0C48(void) {
  game_queue_frontend_cue(0x102);
  D_801D4289 = 4;
  D_801D4286 = D_801D4286 + 1;
}
