#include "bof3/emi/dispatchers.h"

/* This overlay's handler run based at 0x800C26B0: seven code pointers that all
 * lie inside this payload, from 0x800C196C through 0x800C1CD0. The run is the
 * nine-word run based at 0x800C26A8 read one word in, so these are that run's
 * entries 1 through 7, and its trailing words 0x800C26C0 through 0x800C26C8
 * hold this payload's no-op handlers. The next word at 0x800C26CC opens the
 * byte-1 run that callByte1Table26cc indexes.
 * The words themselves sit inside the Splat `textbin` row D_800C2640, so the
 * run is only referenced here. */
extern void (*D_800C26B0[])(void); /* @source 0x800C26B0 @kind data */

/* @source 0x800C1928
 * @behavior Overlay step dispatch on work-record byte 4: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 4, scales it by four and calls the matching
 * entry of this payload's handler run based at 0x800C26B0 with no arguments.
 * The original does not bound the index before the scaled table load. It reads
 * no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps the return address at 0x10 across
 * the indirect call, and the slot after the index byte and the slot after the
 * table load stay nops. The epilogue reloads the return address and the stack
 * pointer without touching $v0, so a caller that reads the result sees whatever
 * the selected handler left there. The only in-image word that names it is
 * 0x800C26A8, index 0 of the nine-word run the already-exact sibling
 * callStateTable26Boss indexes with work byte 3,
 * so the byte-3 dispatcher enters it when work byte 3 is 0: it has no jal
 * caller and takes no arguments. The body differs from that sibling in two
 * words only, the index byte offset, 3 there and 4 here, and the table's %lo
 * word, 0x26A8 there and 0x26B0 here, so this source keeps the sibling's
 * statement order and writes this function's own offset and table binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkByte4State, callWorkByte4State, D_800C26B0);
