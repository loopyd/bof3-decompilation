#include "bof3/emi/dispatchers.h"

/* Interior base of the overlay's pointer run: the 0x40-byte boss028WorkDataAndStateHandlerTable textbin
 * row covers 0x800C2140 through 0x800C217F and the reviewed Splat layout places
 * it whole, so only the entry base this dispatcher indexes is named here. From
 * 0x800C2154 the run holds eleven code pointers: 0x800C1848, 0x801E52E8,
 * 0x801E35D8, 0x801E3638, 0x800C18A0, 0x800C1A10, 0x801E3B68, 0x801E4AE8,
 * 0x801E4D8C, 0x801E4F64 and 0x801E52E8. */
extern void (*D_800C2154[])(void); /* @source 0x800C2154 @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of the eleven-word code pointer run based at 0x800C2154 (0x800C1848,
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x800C18A0, 0x800C1A10, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8) with no arguments; its first
 * entry is the function at 0x800C1848, this overlay's next code span. The
 * original does not bound the index before the scaled table load, and the slots
 * after the index byte, after the table load and in the branch delay slot stay
 * nops. It writes no state and leaves the handler's $v0 untouched; the
 * pointer-cell load precedes frame creation, and the 0x18-byte frame only keeps
 * $ra at 0x10 across the indirect call. No word of this payload holds
 * 0x800C1804, so the caller that reaches this shim sits outside the payload.
 * Seeded from the exact sibling dispatchStateByte1Table2660BossBoss00816_800C1804
 * of boss008/16, which gated exact: the two 0x44-byte spans compare equal word
 * for word except the scaled table load's low immediate, 0x800C2154 here
 * against 0x800C2660 there, so only this target's own table literal is written
 * from these bytes. The first thirteen words of this payload's own 0x800C1C30
 * body are the same walk up to its work-byte-2 index and its 0x800C21AC table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchStateByte1Table2154BossBoss02816_800C1804,
                      D_800C2154);
