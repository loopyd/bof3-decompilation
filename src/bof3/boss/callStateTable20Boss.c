#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the payload's reviewed `textbin` row D_800C2020
 * spans 0x800C2020 through 0x800C20BF and closes the payload. Its first five
 * words are a small record (0x04040000, 0x02020303, 0, 0x06010600, 0xFFFFFFFF)
 * and the words from 0x800C2034 to 0x800C20BC are in-image code pointers, this
 * payload's functions together with shared main-executable handlers. This
 * dispatcher indexes the run's last word 0x800C20BC, which holds 0x800C1EE4, the
 * byte-2 work dispatcher that directly follows this body. The run stays one
 * reviewed row, so this declaration binds only that interior word.
 */
extern void (*D_800C20BC[])(void); /* @source 0x800C20BC @kind data */

/* @source 0x800C1EA0
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at its last word 0x800C20BC, with no
 * arguments. The original does not bound the index before the scaled table load.
 * It reads no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the slots after the index byte and after the table load stay nops. The
 * selected handler's $v0 is left untouched by the epilogue, so this shim returns
 * whatever that handler returns. The payload holds neither a jal nor a pointer
 * word naming 0x800C1EA0, so the overlay loader is its caller. Same shape as the
 * exact sibling callStateTable2080Boss of this target:
 * the two 0x44-byte spans differ in one byte only, the %lo of this dispatcher's
 * table address in the scaled load, and the two addresses sit in the same 0x8000
 * page, so its %hi word is equal too.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable20Boss, D_800C20BC);
