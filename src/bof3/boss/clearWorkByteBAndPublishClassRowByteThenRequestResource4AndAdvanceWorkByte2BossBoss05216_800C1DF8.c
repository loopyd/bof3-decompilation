#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load; the original re-reads
 * the cell before each later store group because a store through the loaded
 * pointer may alias the cell, so every group names its own local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Class byte of the 0x118-stride local panel record run: the leading record's
 * byte selects the 0x88-byte class row read below. Same spelling as the
 * main-executable lift func_801E8DD8, which reads the same run. */
extern u8 D_801EB710[]; /* @source 0x801EB710 @kind unknown */

/* Main-executable resource request helper: the sibling boss lifts of this family
 * declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);
extern void func_8014D978(void);
extern void func_8014D4E0(void);

/* @source 0x800C1DF8
 * @behavior Work-step handler of this overlay. Loads the scratch work record
 * published at the pointer cell 0x1F800044, clears its byte 0x0B, re-reads the
 * cell and stores byte 0x52 of the 0x88-byte class row selected by the leading
 * class byte of D_801EB710 into record byte 0x09, requests resource 4 through
 * func_8014D6B8, runs the shared battle updates func_8014D978 and func_8014D4E0
 * in that order, then re-reads the cell once more and advances record byte 0x02
 * by one. The class rows are based at 0x800E407C, so the byte read is
 * 0x800E40CE + class * 0x88, the same row family the main-executable lift
 * func_801E8DD8 reads at 0x800E40CB. The byte 0x09 store is the selector the
 * next entry of this payload's handler run, 0x800C1E7C, tests against 0xFF and
 * 0, so byte 0x09 arms that follow-up step. No jal in this payload targets
 * 0x800C1DF8; the only word equal to its address is the payload word at
 * 0x800C1804, the first of the three-word in-image address run 0x800C1804
 * through 0x800C180C, so a dispatcher outside the payload or that run selects
 * it. Takes no arguments and returns nothing; the 0x18-byte frame keeps $ra at
 * 0x10 across the calls and the pointer cell is read before frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: the class-row read is written as an address constant plus the
 * scaled index, the form the exact lift initEnemyCueCountdownWhenLoaderReady
 * uses for the same 0x88-byte rows at 0x800E40CF. Declaring the row base as a
 * named array and indexing it makes the assembler expand the symbolic
 * displacement and emit `addu at, at, idx`, which is not this function's
 * `addu at, idx, at`; the address-constant form is measured against the original
 * with no other change, and it reproduces the original instruction.
 */
void clearWorkByteBAndPublishClassRowByteThenRequestResource4AndAdvanceWorkByte2BossBoss05216_800C1DF8(
    void) {
  u8 *work;
  u8 *slot;
  u8 *step;

  work = D_1F800044;
  work[0xB] = 0;
  slot = D_1F800044;
  slot[0x9] = PSX_REF(u8, 0x800E40CEu + D_801EB710[0] * 0x88u);
  func_8014D6B8(4);
  func_8014D978();
  func_8014D4E0();
  step = D_1F800044;
  step[0x2]++;
}
