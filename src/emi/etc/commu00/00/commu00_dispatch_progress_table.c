#include "internal.h"

/* @source 0x801F16BC
 * @behavior dispatches through the local six-entry jump table
 *           commu00_progress_handlerTable, indexed by the signed byte commu00_fairy_progress[0].
 */
void commu00_dispatch_progress_table(void)
{
  commu00_progress_handlerTable[(s8)commu00_fairy_progress[0]]();
}
