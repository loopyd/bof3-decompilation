#include "bof3/emi/dispatchers.h"

/* This overlay's step-handler run: the thirteen words from 0x800C1804 through
 * 0x800C1837 are in-image code pointers, this payload's step handlers, and the
 * 0x800C1804 `textbin` row carries the whole run. func_800C19C4 copies its
 * first eight words into a frame table and then reads entries 8 and 9
 * directly, so the run is only referenced here. This dispatcher indexes a
 * second run, the twelve handler words from 0x800C2090 through 0x800C20BC that
 * sit inside the 0x800C2078 `textbin` row; its entry 0 is this payload's
 * 0x800C187C and entries 1 through 11 are main-executable handlers. */
extern void (*D_800C2090[])(void); /* @source 0x800C2090 @kind data */

/* @source 0x800C1838
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, opens the
 * 0x18-byte frame that keeps $ra at 0x10 across the indirect call, then takes
 * that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of this overlay's handler run based at 0x800C2090 with no
 * arguments. The chosen entry is read from the table itself, not from a frame
 * copy, and the original does not bound the index before the scaled table
 * load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation and the slots after the index byte and after the
 * table load stay nops. The epilogue restores $ra and $sp without touching
 * $v0, so the selected handler's return value survives. The payload never
 * names this dispatcher, neither as a jal target nor as a stored word, so the
 * overlay loader that steps the work record is its caller: it has no jal
 * caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2090BossBoss05416_800C1838, D_800C2090);
