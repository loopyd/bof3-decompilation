#include "bof3/world/area03004_internal.h"

/* @behavior Step the active-work sequence. When it ends or redirects, select sequence 3
 * and advance the dispatch index. Project and queue the active work on both
 * paths.
 * @source 0x801D25F0
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerWhenSequenceEndsOrRedirectsWorld00Area03004(void) {
  if ((u8)stepActiveWorkSequence() != 0u) {
    u8* cursor;

    selectActiveWorkSequence(3);
    cursor = (u8*)D_1F800044;
    cursor[1] = (u8)(cursor[1] + 1u);
  }
  projectAndQueueActiveWork();
}
