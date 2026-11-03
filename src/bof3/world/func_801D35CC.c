#include "bof3/world/area03004_internal.h"

/* @behavior dispatches the scratch work-record index at offset 1 through the handler
 * table at 0x801E1F44 while the record published at the scratchpad cursor is the
 * shared record at 0x80144124 or the shared mode byte has already reached four;
 * otherwise it runs the shared func_80196070 helper.
 * @source 0x801D35CC
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D35CC(void) {
  if ((D_1F800044 != D_80144124) && (modeByte < 4u)) {
    func_80196070();
  } else {
    D_801E1F44[D_1F800044[1u]]();
  }
}
