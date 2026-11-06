#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the fifteen in-image code-pointer words 0x801F4D58
 * through 0x801F4D93. This dispatcher indexes the run from 0x801F4D84, so its base
 * is entry 11 of that data and its reachable entries are 0x801F3594, 0x801F3614,
 * 0x801F36EC and 0x801F37EC. Its own address is not a word of the run: the only
 * in-image reference to it is the `jal` at 0x801F3374 inside
 * invokeTwoHelpersWorld04Area15213_801F3364, which calls it after the byte-2
 * dispatcher. */
extern void (*D_801F4D84[])(void); /* @source 0x801F4D84 @kind data */

/* @source 0x801F3550
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once, takes
 *           that record's unsigned byte at offset 3, scales it by four and calls the
 *           matching entry of this overlay's handler run based at 0x801F4D84 with no
 *           arguments, discarding any result. The original does not bound the index
 *           before the scaled table load. It reads no other state and writes none;
 *           the pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the indirect call, and the slot after the index
 *           byte and the slot after the table load stay nops. It takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchStateByte3Table4d84World04Area15213_801F3550, D_801F4D84);
