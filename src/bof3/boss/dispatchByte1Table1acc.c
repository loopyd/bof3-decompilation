#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's pointer run: the words from 0x800C1ACC
 * through 0x800C1B10 sit inside the 0x800C1AB8 textbin row this payload keeps
 * for its last 0x68 bytes, and the reviewed Splat layout places that row whole,
 * so only the entry base this dispatcher indexes is named here. From 0x800C1ACC
 * the eighteen words this dispatcher selects from are the code pointers
 * 0x800C1848, 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00,
 * 0x800C18A0, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8, 0x801E5288,
 * 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490, 0x800C18E4 and 0x801E4928.
 * Entries 0, 6 and 16 are this payload's own spans at 0x800C1848, 0x800C18A0
 * and 0x800C18E4; the other fifteen lie outside the payload. The three words
 * from 0x800C1B14 belong to the byte-indexed dispatcher at 0x800C1978, not to
 * this table. */
extern void (*D_800C1ACC[])(void); /* @source 0x800C1ACC @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of the eighteen-word code pointer run based at 0x800C1ACC with no
 * arguments, leaving the handler's $v0 in the return register. It writes no
 * state and reads no other cell. The original does not bound the index before
 * the scaled table load, the slots after the index byte, after the table load
 * and in the branch delay slot stay nops, the pointer-cell load precedes frame
 * creation, and the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call. No word of this payload holds 0x800C1804 and no jal reaches it, so the
 * caller that steps this overlay sits outside the payload and the entry takes
 * no arguments; it is this payload's first code span, directly behind the four
 * header bytes and ahead of the step-0 span at 0x800C1848.
 *
 * Byte evidence: the 0x44-byte span of the sibling
 * dispatchWorkByte1Table1B2CBossBoss02213_800C1804 at the same address of
 * boss022/13 compares equal word for word except the scaled table load's word,
 * 0x800C1B2C there against 0x800C1ACC here, so the statement order, operand
 * forms and declaration style follow that exact sibling while the table literal
 * is written from these bytes. The shared dispatcher definition emits the same
 * seventeen instructions.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchByte1Table1acc,
                      dispatchByte1Table1acc,
                      D_800C1ACC);
