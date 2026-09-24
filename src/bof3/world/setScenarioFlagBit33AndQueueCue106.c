#include "bof3/world/area02613_internal.h"

/* @source 0x801F31EC
 * @behavior runs the scenario reward gate for the (4, 3) ability id, queues frontend cue 0x106, then sets bit 0x33 in the active scenario flags.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit33AndQueueCue106(void) {
  func_801650B4(4, 3, 1, 0);
  func_8015DF18(0x106u);
  func_8015B580((void*)D_8014686C, 0x33u);
}
