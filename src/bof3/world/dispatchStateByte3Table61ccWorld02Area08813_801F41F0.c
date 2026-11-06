#include "bof3/emi/dispatchers.h"

extern void (*D_801F61CC[])(void);

/* @source 0x801F41F0
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044, takes that
 *           record's unsigned byte at offset 3, scales it by four and calls the
 *           matching entry of the four-pointer handler run based at 0x801F61CC: the
 *           words 0x801F4234, 0x801F42B4, 0x801F438C and 0x801F448C. The call takes
 *           no arguments and no result is used. The original does not bound the
 *           index before the scaled table load. It reads no other state and writes
 *           none; the pointer-cell load precedes frame creation, the 0x18-byte frame
 *           only keeps $ra at 0x10 across the indirect call, and the slots after the
 *           index byte and after the table load stay nops. It takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchStateByte3Table61ccWorld02Area08813_801F41F0, D_801F61CC);
