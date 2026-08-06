#include "internal.h"

/* @source 0x801F18BC
 * @behavior dispatches through the local six-entry offset jump table
 *           commu00_progress_handlerTable, indexed by the signed byte commu00_fairy_progress[0].
 */
void commu00_dispatch_progress_table_alt(void)
{
  commu00_progress_handlerTable[(s8)commu00_fairy_progress[0] + 6]();
}
