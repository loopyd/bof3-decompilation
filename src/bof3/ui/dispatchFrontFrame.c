#include "bof3/ui/game00_internal.h"

/**
 * @source 0x80199368
 * @behavior runs the scenario-local handler/state dispatch through
 * callSceneHandlerState, then the shared update slice func_801D1740,
 * then finalizes one shared front-end frame through func_80158C80.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchFrontFrame(void) {
  callSceneHandlerState();
  func_801D1740();
  func_80158C80();
}
