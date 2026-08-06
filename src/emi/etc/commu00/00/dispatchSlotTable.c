#include "internal.h"

/* @source 0x801F1F10
 * @behavior dispatches through the local offset jump table slotHandlerTable,
 *           indexed by the unsigned byte fairySlotIndex.
 */
void dispatchSlotTable(void)
{
  slotHandlerTable[fairySlotIndex]();
}
