#include "bof3/emi/dispatchers.h"

/* This overlay's shared data run starts at 0x800C1F3C and holds five packed
 * byte/word slots followed by twenty-two code pointers from 0x800C1F50 to
 * 0x800C1FA4, mixing this payload's functions with the shared main-executable
 * handlers at 0x801E.... The whole run sits inside the 0x800C1F3C `textbin` row,
 * which the Splat layout places, so it is only referenced here. This dispatcher
 * indexes it from that run's word 12, 0x800C1F80, and the run ends at its word 21,
 * 0x800C1FA4, so the ten words from the base reach the main-executable handlers
 * 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490 and 0x801E4928 interleaved with
 * this payload's own functions 0x800C18FC, 0x800C19BC, 0x800C19C4, 0x800C19CC and
 * 0x800C1D98; any higher work-record byte reads past the payload. */
extern void (*D_800C1F80[])(void); /* @source 0x800C1F80 @kind data */

/* @source 0x800C18B8
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 2, scales it by four and calls the
 * matching entry of this overlay's handler table based at 0x800C1F80, forwarding
 * no arguments. The entry is read from the table itself, not from a frame copy.
 * The original does not bound the index before the scaled table load, and the
 * slots after the index byte and after that load stay nops. The pointer-cell load
 * precedes frame creation; the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the epilogue leaves the handler's $v0 untouched, so this
 * shim's result is whatever that handler returns. The payload's only word naming
 * 0x800C18B8 is 0x800C1F68, word 6 of the same code-pointer run, and no jal site
 * in the payload targets it, so its caller is outside this overlay. Same body as
 * the already-exact sibling dispatchStateByte1Table1fa4BossBoss05216_800C1D54,
 * which differs in the work-record byte it reads (0x01 there, 0x02 here) and in
 * the table base (0x800C1FA4 there, 0x800C1F80 here).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table1f80BossBoss05216_800C18B8, D_800C1F80);
