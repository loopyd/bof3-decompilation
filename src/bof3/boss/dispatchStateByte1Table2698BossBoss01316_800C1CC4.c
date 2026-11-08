#include "bof3/emi/dispatchers.h"

/* This overlay's handler run inside the reviewed `textbin` row D_800C25D8: the
 * row's words 18 through 66, starting at 0x800C2620, hold this payload's own step
 * entries 0x800C1860, 0x800C1934, 0x800C1A04, 0x800C1A2C, 0x800C1A78, 0x800C1B4C,
 * 0x800C1C4C, 0x800C1CBC, 0x800C1D08, 0x800C1D58, 0x800C1D9C, 0x800C1DF4,
 * 0x800C1E90, 0x800C1F24 and 0x800C2004 beside the shared main-executable
 * handlers 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E51DC, 0x801E5288 and 0x801E52E8,
 * with the zero word 0x800C26E0 last; the row's first eighteen words hold other
 * payload data. Several dispatchers of this payload index the run from their own
 * interior word, so Splat places the whole row once and it is only referenced
 * here. This dispatcher indexes from 0x800C2698, the run's entry 30, whose entry 0
 * is this payload's next step 0x800C1D08. */
extern void (*D_800C2698[])(void); /* @source 0x800C2698 @kind data */

/* @source 0x800C1CC4
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of this overlay's handler run based at 0x800C2698 with no
 * arguments. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after the table load stay nops. It reads
 * no other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim returns
 * whatever that handler returns. No word or jal in this payload names 0x800C1CC4,
 * and it is no entry of the handler run either, so the overlay loader is what
 * calls it. The 68-byte body is instruction-for-instruction identical to the
 * already-exact siblings dispatchStateByte1Table2744BossBoss00816_800C1EB0,
 * dispatchStateByte1Table1ddcBossBoss00716_800C190C and
 * dispatchStateByte1Table0c60BossBoss03816_800C193C, differing only in the %lo
 * table displacement of the scaled-index load.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchStateByte1Table2698BossBoss01316_800C1CC4,
                      D_800C2698);
