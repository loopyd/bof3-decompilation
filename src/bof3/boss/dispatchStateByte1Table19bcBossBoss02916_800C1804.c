#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's pointer run: the words from 0x800C19BC
 * through 0x800C19F4 sit inside the 0x800C19A8 asm row this payload keeps for
 * its last code span, and the reviewed Splat layout places that row whole, so
 * only the entry base this dispatcher indexes is named here. From 0x800C19BC
 * its own table holds twelve code pointers: 0x800C1848, 0x801E52E8,
 * 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8,
 * 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288. The first entry is the
 * function at 0x800C1848, this overlay's next code span; the remaining eleven
 * are outside the payload. The three words from 0x800C19EC belong to the
 * byte-indexed dispatcher at 0x800C18A0, not to this table. */
extern void (*D_800C19BC[])(void); /* @source 0x800C19BC @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of the twelve-word code pointer run based at 0x800C19BC with no
 * arguments, leaving the handler's $v0 in the return register. It writes no
 * state and reads no other cell. The original does not bound the index before
 * the scaled table load, the slots after the index byte, after the table load
 * and in the branch delay slot stay nops, the pointer-cell load precedes frame
 * creation, and the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call. No word of this payload holds 0x800C1804 and no jal reaches it, so the
 * caller that steps this overlay sits outside the payload and the entry takes
 * no arguments. Seeded from the shape sibling
 * dispatchStateByte1Table2154BossBoss02816_800C1804 of boss028/16, which gated
 * exact: the two 0x44-byte spans compare equal word for word except the scaled
 * table load's low immediate, 0x800C2154 there against 0x800C19BC here, so only
 * this target's own table literal is written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchStateByte1Table19bcBossBoss02916_800C1804,
                      D_800C19BC);
