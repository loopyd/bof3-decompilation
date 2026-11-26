#include "bof3/emi/dispatchers.h"

typedef void (*Boss05216Handler)(void);

/* This overlay's handler run starts at 0x800C1F50, the first of the twenty-two
 * code pointers that follow the five packed byte/word slots of the 0x800C1F3C
 * `textbin` row, and its last entry sits at 0x800C1FA4, so the whole run is
 * placed by the reviewed Splat layout and only referenced here. Word 0 of the
 * run is this payload's own 0x800C1854, the overlay init that publishes the run
 * base at record offset 0xEC. The remaining entries interleave this payload's
 * handlers 0x800C18FC, 0x800C18B8, 0x800C19BC, 0x800C19C4, 0x800C19CC and
 * 0x800C1D98 with the shared main-executable handlers 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, 0x801E3BAC, 0x801E3BD0, 0x801E4490, 0x801E4928,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E5288 and 0x801E52E8. */
extern Boss05216Handler D_800C1F50[]; /* @source 0x800C1F50 @kind data */

/* The twelve bytes 0x800C1804 through 0x800C180F, immediately before this
 * dispatcher, are this overlay's three-entry step-handler table: 0x800C1DF8,
 * 0x800C1E7C and 0x800C1F1C, in that order. They are the whole of the reviewed
 * `textbin` row D_800C1804, which ends where this payload's `c` row begins at
 * 0x800C1810, so the row is placed by the Splat layout and only referenced
 * here. The payload's consumer is the still unlifted step dispatcher
 * func_800C1D98, which materializes that base with a lui/addiu pair, reads all
 * three words into its 0x28-byte frame and calls the entry selected by
 * work-record byte 2; the run has no other reader. */
extern Boss05216Handler D_800C1804[]; /* @source 0x800C1804 @kind data */

/* @source 0x800C1810
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044 once,
 * takes that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of this overlay's handler run based at 0x800C1F50, forwarding
 * no arguments. The selected entry is read from the run itself rather than from
 * a frame copy, and the original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, the epilogue leaves the selected handler's $v0 untouched, so
 * this shim's result is whatever that handler returns, and the slots after the
 * index byte and after the table load stay nops. No word of this payload holds
 * 0x800C1810 and no jal names it, so the overlay loader that steps this payload
 * is its caller and the function takes no arguments. Same 17-instruction,
 * 68-byte body as the exact sibling
 * dispatchStateByte1Table1b4cBossBoss05016_800C1810, which differs only in the
 * table its lui/addu/lw triple materializes, so that sibling supplied the
 * statement order and this body keeps its own 0x800C1F50 literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateByte1Table5, D_800C1F50);
