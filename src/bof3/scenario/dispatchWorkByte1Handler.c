#include "bof3/scenario/scena00_internal.h"
#include "bof3/emi/dispatchers.h"

/* @source 0x801F6DC0
 * @behavior Dispatches the overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer cell at 0x1F800044, takes its state
 * byte at offset 1, and invokes that index of the local handler table at
 * 0x801FC980 (the table holds the overlay's own handler entries); it reads no
 * other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1Handler, D_801FC980);
