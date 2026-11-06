#include "bof3/bof3.h"

typedef void (*BmagicMagic05003StepHandler)(void);

/*
 * Three-entry step-handler table at 0x801EFAF0, the word this function's own
 * scaled index is added to: entry 0 = 0x801EEFDC, entry 1 = 0x801EF0E8,
 * entry 2 = 0x801EF138. The words sit inside the reviewed data row that starts
 * at 0x801EFA90, and the byte-2 load below is the only in-image reference.
 */
extern BmagicMagic05003StepHandler D_801EFAF0[]; /* @source 0x801EFAF0 @kind table */

/*
 * Scratchpad work-record pointer cell at 0x1F800044. The cell is read twice
 * per frame, once before the step-handler call and once after it, and this
 * function's bytes show each read built from its own lui plus offset load, so
 * the cell is bound by name rather than through the constant-address
 * accessor: the named cell re-materialises its address per access, while the
 * accessor's shared address constant stays live across the call in a
 * callee-saved register and adds eight bytes to the frame.
 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Update passes run for a frame whose work record is in an active state. */
void func_800AF764(void);
void func_801EF1B0(void);
void func_801EF4C4(void);
void func_801EF898(void);

/* @source 0x801EEF4C
 * @behavior Handler installed as entry 0 of the byte-1 dispatch table at
 *   0x801EFA9C. Reads the work-record pointer from the scratchpad cell at
 *   0x1F800044, takes the step handler selected by the record byte at offset 2
 *   from the three-entry table at 0x801EFAF0 and calls it with no arguments,
 *   then re-reads the work-record pointer after that call and, when the record
 *   byte at offset 2 and the record byte at offset 0 are both nonzero, calls
 *   func_800AF764, func_801EF1B0, func_801EF4C4 and func_801EF898 in that
 *   order. It returns nothing and writes no state of its own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2ThenCallUpdatePassesBmagicMagic05003_801EEF4C(void) {
  D_801EFAF0[D_1F800044[2]]();
  if (D_1F800044[2] != 0 && D_1F800044[0] != 0) {
    func_800AF764();
    func_801EF1B0();
    func_801EF4C4();
    func_801EF898();
  }
}
