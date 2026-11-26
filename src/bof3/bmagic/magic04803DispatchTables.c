#include "bof3/emi/dispatchers.h"
#include "bof3/bmagic/magic04803_internal.h"

/* @source 0x801EEE70
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF818. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTablef818, callStateTablef818, workByte1HandlerTable);
