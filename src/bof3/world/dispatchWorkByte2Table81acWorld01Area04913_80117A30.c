#include "bof3/emi/dispatchers.h"

/* This overlay's handler pointer run starts at 0x8011818C and holds thirteen
 * in-image code pointers, 0x8011818C through 0x801181BC. This dispatcher indexes
 * it from 0x801181AC, so the base is entry 8 and the five reachable entries are
 * func_80117A74, func_80117ACC, func_80117C38, func_80117C58 and func_80117DF0,
 * the last word of the run. The Splat `textbin` row D_8011818C places the words,
 * so the table is only referenced here. */
extern void (*D_801181AC[])(void); /* @source 0x801181AC @kind data */

/* @source 0x80117A30
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x801181AC with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the slot after the index byte and the slot after the
 * table load stay nops. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2Table81acWorld01Area04913_80117A30, D_801181AC);
