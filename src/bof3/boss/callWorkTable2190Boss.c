#include "bof3/emi/dispatchers.h"

/* This overlay's in-image code pointer run fills 0x800C2180 through 0x800C21C7:
 * the reviewed `textbin` row T_800C2180 holds its first sixteen words and the
 * row labelled T_800C21C0 its last two, which are code pointers too. The run
 * is only referenced here. This dispatcher indexes it from the word 0x800C2190,
 * an interior word of it, so the run stays two reviewed rows and this
 * declaration binds only that word. The entries it reaches from there, up to the
 * run's final word, are 0x800C1A54, 0x800C1AA0, 0x800C1B28, 0x800C1B30,
 * 0x800C1B38, 0x800C1C30, 0x800C1FB8, 0x800C1C9C, 0x800C1D90, 0x800C1E80,
 * 0x800C1F2C, 0x800C2024, 0x800C20C8 and 0x800C2120.
 */
extern void (*D_800C2190[])(void); /* @source 0x800C2190 @kind data */

/* @source 0x800C1A10
 * @behavior Overlay work dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's code pointer run based at 0x800C2190 with no
 * arguments. The original does not bound the index before the scaled table load.
 * It reads no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * the slot after the index byte and the slot after the table load stay nops, and
 * the epilogue leaves the selected handler's $v0 untouched, so this shim returns
 * whatever that handler returns. No jal names 0x800C1A10: its address appears
 * once in this payload, as the word at 0x800C2168 inside the 0x800C2140 data
 * row, so the main program installs it as a callback. Same shape as the sibling
 * callWorkTable2184Boss, whose body differs only in
 * the scaled table load's low immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable2190Boss, D_800C2190);
