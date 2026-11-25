#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's handler run. The reviewed 0x800C1AB8 `textbin`
 * row covers 0x800C1AB8 through the payload end at 0x800C1B20. The eighteen-word
 * code-pointer run the dispatchers of this target index starts at 0x800C1ACC,
 * five words into that row, and reaches 0x800C1B14, where the three inert
 * handlers of the byte-indexed dispatcher begin. This dispatcher indexes the run
 * from its word 12, 0x800C1AFC, so that base word is named here on its own while
 * the reviewed row stays whole; the byte-1 dispatcher names word 0 of the same
 * run. The six words from 0x800C1AFC are 0x801E3BAC, 0x801E3BD0, 0x801E3BAC,
 * 0x801E4490, 0x800C18E4 and 0x801E4928: entry 4 is the next code span of this
 * overlay and the other five lie outside the payload. */
extern void (*D_800C1AFC[])(void); /* @source 0x800C1AFC @kind data */

/* @source 0x800C18A0
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record byte at offset 0x02 as the index, scales it by
 * four and calls the matching entry of the six-word code-pointer run based at
 * 0x800C1AFC with no arguments. The original does not bound the index before the
 * scaled table load, and the slots after the index byte, after the table load and
 * in both branch delay slots stay nops. It writes no state and leaves the
 * handler return register untouched; the 0x18-byte frame only keeps the return
 * address at 0x10 across the indirect call. The only payload word that names this
 * address is 0x800C1AE4, word 6 of the run based at 0x800C1ACC that
 * dispatchByte1Table1acc indexes, and no jal in this
 * payload names it, so that shim is its caller: it passes no arguments and
 * forwards none.
 *
 * Byte evidence: the other span of this payload that carries the same body,
 * 0x800C1804, compares equal word for word across all seventeen instructions
 * except the index byte load, which reads offset 0x01 there and 0x02 here, and
 * the scaled table load low word, 0x800C1ACC there and 0x800C1AFC here. The
 * shared dispatcher definition reproduces that body, so the statement order,
 * operand forms and declaration style come from the seed while both literals
 * above are written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2,
                      dispatchByte2Table1afc,
                      D_800C1AFC);
