#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

/* The three payload words at 0x800C184C are 0x800C305C, 0x800C3134 and
 * 0x800C31F0, so the table holds this overlay's three alternative step bodies
 * for the work record's byte +2. */
typedef struct {
  Boss05516Handler entry[3];
} Boss05516StepTable;

extern Boss05516StepTable D_800C184C; /* @source 0x800C184C @kind data */

/* @source 0x800C2FFC
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: copies the three payload
 * handler words at 0x800C184C (0x800C305C, 0x800C3134, 0x800C31F0) into frame
 * slots 0x10, 0x14 and 0x18 and calls the entry selected by the record's
 * unsigned byte +2 without arguments. The original reads the scratchpad
 * pointer before it adjusts the stack and copies the table as one aggregate,
 * three loads ahead of three stores, and it leaves the index unbounded: the
 * 0x28-byte frame holds nothing but the copied table and $ra, and the function
 * writes no state, so the selected entry owns the rest of the step. The word
 * at 0x800C3610 holds this function's address and no other word of the payload
 * does, so it is entry 9, the last entry, of the 0x800C35EC handler run this
 * overlay indexes, following 0x800C2E74 at entry 8. The selected entries are
 * alternative step bodies: 0x800C305C copies the 0xA-byte template at
 * 0x800C1858 into its frame, calls 0x8014DD3C with 0x318, clears work-record
 * bytes +0x48 and +0x24 and word +0x3C, runs
 * 0x8014D6B8/0x801E30F8/0x8014D4E0/0x800C3284 and finally advances byte +2 of
 * the same record by one, so the indexed byte selects which of the three step
 * bodies runs next.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2TableCopyBossBoss05516_800C2FFC(void) {
  Boss05516StepTable table = D_800C184C;

  table.entry[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
