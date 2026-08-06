#include "internal.h"

/* @source 0x801F1E74
 * @behavior dispatches through the local offset jump table commu00_progress_handlerTable3,
 *           indexed by the signed byte commu00_fairy_progress[0].
 */
void commu00_dispatch_progress_table3(void)
{
  commu00_progress_handlerTable3[(s8)commu00_fairy_progress[0]]();
}
