#include "bof3/emi/dispatchers.h"

extern void (*D_8011818C[])(void);

/* @source 0x80117004
 * @behavior Overlay step dispatch of this target: reads byte +1 of the work
 * record published at the scratchpad pointer cell 0x1F800044 and calls the
 * handler at that index of the overlay's unchecked handler table based at
 * 0x8011818C, a run of thirteen in-image pointers from the boot handler
 * func_80117048 (word 0) through func_80117DF0 (word 12); word 1 is the state-1
 * handler func_801170A4 and the word after the run is not a handler. The record
 * pointer is loaded before the frame is created. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1Table818cWorld01Area04913_80117004, D_8011818C);
