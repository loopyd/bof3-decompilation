#include "bof3/bof3.h"

/* The overlay's own record-table cursor. The original materialises the cell
 * address and reloads the published pointer for every access below, and this
 * target's tick tickAndDrawActiveWorkRecordsWorld03Area13213_801F38B0 walks the
 * same 0x18-byte-stride table at 0x800E4800 through this same cell. */
extern u8* D_801F4A88; /* @source 0x801F4A88 @kind data */

/* @source 0x801F385C
 * @behavior Table reset helper: publishes the base 0x800E4800 of the
 *           0x18-byte-stride work table in main RAM into this overlay's own
 *           cursor cell 0x801F4A88, then walks the 0x20 records of that table
 *           with an unsigned byte counter in a bottom-tested loop, clearing the
 *           claim byte at offset 0 of each record and advancing the published
 *           cursor by 0x18 per record, so the cell ends one stride past the
 *           last record. It takes no arguments, returns nothing and reads no
 *           other state.
 *
 * Field evidence: the offset-0 byte this walk clears is the record claim byte.
 * The lift in src/bof3/world/tickAndDrawActiveWorkRecordsWorld03Area13213_801F38B0.c
 * reads the byte at offset 0 of every 0x18-stride record through the same cell
 * as the claim that marks the record active and clears it when the countdown at
 * +2 expires, and the lift in
 * src/bof3/world/seedWorkRecordDefaultsAndWorkPositionWorld03Area13213_801F37A4.c
 * documents the same 0x20-entry table for the same stride.
 *
 * Caller evidence: both in-payload call sites are `jal` at 0x801F2E40 in
 * func_801F2E38 and at 0x801F2FC4 in func_801F2F98, each with a `nop` delay
 * slot and no argument setup, so the parameters stay structural and the
 * function is declared void (void).
 *
 * Shape evidence: this is the repository's work-table walk family; the already
 * matched clearWorkTableAt800E4800ScenarioScena0100_801F7AB8 and
 * clearWorkRecordCountdownsAt800E4800World01Area06714_801F39AC clear one field
 * of a table at the same base with an unsigned byte counter in the same order,
 * and each carries its own stride, count and stored field, which are written
 * here from this span's own original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableClaimBytesWorld03Area13213_801F385C(void) {
  u8 index;

  D_801F4A88 = PSX_PTR(u8, 0x800E4800u);
  index = 0u;
  do {
    D_801F4A88[0] = 0;
    D_801F4A88 += 0x18;
    index += 1u;
  } while (index < 0x20u);
}
