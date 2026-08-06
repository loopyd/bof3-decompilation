#include "internal.h"

/* @source 0x801F1F10
 * @behavior dispatches through the local offset jump table commu00_slot_handlerTable,
 *           indexed by the unsigned byte commu00_fairy_slot_index.
 */
void commu00_dispatch_slot_table(void)
{
  commu00_slot_handlerTable[commu00_fairy_slot_index]();
}
