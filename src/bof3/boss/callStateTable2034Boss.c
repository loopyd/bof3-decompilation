#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the payload's reviewed `textbin` row D_800C2020
 * spans 0x800C2020 through 0x800C20BF. Its first five words are a small record
 * (0x04040000, 0x02020303, 0, 0x06010600, 0xFFFFFFFF) and the thirty-five words
 * from 0x800C2034 to 0x800C20BC are in-image code pointers, this payload's
 * functions together with the shared main-executable handlers at 0x801E......,
 * starting with 0x800C1854. This dispatcher indexes that run from its own first
 * pointer word 0x800C2034, the row's sixth word, so the run stays one reviewed
 * row and this declaration binds only that word.
 */
extern void (*D_800C2034[])(void); /* @source 0x800C2034 @kind data */

/* @source 0x800C1810
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C2034 with no arguments. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation, the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the slots
 * after the index byte and after the table load stay nops. The selected
 * handler's $v0 is left untouched by the epilogue, so this shim returns whatever
 * that handler returns. The payload holds neither a jal nor a pointer word naming
 * 0x800C1810, so the overlay loader that steps the handler run is its caller.
 * Seeded from the sibling callStateTable2080Boss of
 * this target, whose recorded measurement is exact: the two spans are equal word
 * for word except the single %lo word of the scaled table load at 0x800C1834,
 * 0x800C2034 here against 0x800C2080 there, both in the one 0x8000 page.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable2034Boss, callStateTable2034Boss, D_800C2034);
