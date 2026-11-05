#include "bof3/world/area03004_internal.h"
#include "bof3/emi/dispatchers.h"

extern void (*stateByte3HandlersWorld00Area03004_801E1FD4[])(void); /* @source 0x801E1FD4 @kind data */

/* @source 0x801D576C
 * @behavior Reads byte 3 of the work record published at scratchpad cell
 * 0x1F800044 and calls the indexed handler in the local table at 0x801E1FD4.
 * No arguments are prepared and the handler result is discarded.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchStateByte3Table1fd4World00Area03004_801D576C, stateByte3HandlersWorld00Area03004_801E1FD4);
