#include "bof3/bof3.h"

/* Thirty-two-slot byte table of the shared main-executable data segment based at
 * 0x8014544C. This body walks it as a flat byte list, so the table takes its
 * declared element type and no stride is applied. The lift in
 * emi/scenario/scena08/00 at 0x801FE12C (still an extracted disassembly, so its
 * shape is original bytes) walks the same table and writes 0x0E back over every
 * slot that holds 4, which fixes 0x0E as the released marker and 4 as the taken
 * one. */
extern u8 D_8014544C[];

/* Main-EXE selector setter, outside this payload and unlifted: the write half of
 * the query pair whose read half 0x80166140 the sibling overlays test with the
 * same ids, so only the call-site prototype is declared here. */
void func_80166180(s32 arg0);

/* @source 0x801F2F0C
 * @behavior Overlay slot claim on the shared 32-slot byte table at 0x8014544C.
 *           The body takes the table base once, then walks it with a pointer
 *           that advances one byte per pass while a byte counter runs 0 through
 *           0x1F alongside it; the original keeps both: the pointer feeds each
 *           `lbu`, and the counter, masked to a byte and compared unsigned
 *           against 0x20, drives the loop test. The first slot whose byte reads
 *           0x0E is overwritten with 4 in the branch delay slot of a jump to the
 *           epilogue, so the scan leaves the loop at that slot and the release
 *           marker becomes the taken one. When all 32 slots carry something else
 *           the scan falls out of the loop and the body sets shared selector id
 *           4 through the main-EXE setter func_80166180 instead, then returns
 *           along the same epilogue. It takes no arguments, reads no state
 *           besides the table and builds a 0x18-byte frame only to keep the
 *           return address across the setter call.
 *
 * Name evidence: the first-free-0e-slot claim, the 4 it writes, the 32-slot
 * bound at 0x8014544C and the fallback selector id 4 all come from this span's
 * own bytes; the table's released/taken marker pairing comes from the
 * 0x801FE12C disassembly of emi/scenario/scena08/00 that rewrites 4 to 0x0E
 * across the same 32 slots; "shared selector" is the role the lifted siblings of
 * this target record for func_80166180 at its other call sites. No payload word
 * names this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFirstFree0eSlotInTable8014544cAsFourElseSetSharedSelector4World01Area03913_801F2F0C(
    void) {
  u8 i;
  u8* slot;

  slot = D_8014544C;
  for (i = 0; i < 0x20; i++) {
    if (*slot == 0x0e) {
      *slot = 4;
      return;
    }
    slot++;
  }
  func_80166180(4);
}
