#include "bof3/emi/dispatchers.h"

/* This overlay's handler run based at 0x800C26A8: nine code pointers that all
 * lie inside this payload. 0x800C26A8 is entry 6 of the fifteen-word run based
 * at 0x800C2690, the run callStateTable2690Boss
 * indexes with work byte 2, so this base covers that run's entries 6 through 14
 * and its own entry 8, 0x800C1CD0, sits at 0x800C26C8, one word before the word
 * at 0x800C26CC where the byte-1 run begins. The words themselves sit inside the
 * Splat `textbin` row D_800C2640, so the run is only referenced here. */
extern void (*D_800C26A8[])(void); /* @source 0x800C26A8 @kind data */

/* @source 0x800C18E4
 * @behavior Overlay step dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this payload's handler run based at 0x800C26A8 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load. It reads no other state and writes none; the pointer-cell
 * load precedes frame creation, the 0x18-byte frame only keeps the return
 * address at 0x10 across the indirect call, and the slot after the index byte
 * and the slot after the table load stay nops. The only in-image word that
 * names it is 0x800C26A0, index 4 of the run based at 0x800C2690, so the
 * byte-2 dispatcher callStateTable2690Boss enters
 * it when work byte 2 is 4: it has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable26Boss, callStateTable26Boss, D_800C26A8);
