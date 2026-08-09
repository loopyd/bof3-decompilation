#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1F10
 * @behavior dispatches through the local offset jump table slotHandlerTable,
 *           indexed by the unsigned byte fairySlotIndex.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSlotTable(void)
{
  slotHandlerTable[fairySlotIndex]();
}
