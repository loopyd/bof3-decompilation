#include "bof3/bof3.h"

/* Shared route word of the main executable: the step id this overlay's stage
 * gates key on, read here with five `lhu` loads (one per route test).
 * @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
/* Sub-step byte of the same route tuple, read with two `lbu` loads: route 0x28
 * requires 7 and route 0x30 requires 3.
 * @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;
/* Shared scenario bank word of the main executable, handed to the stage wait
 * and stage commit helpers; the original loads it with one `lw` before every
 * one of those calls.
 * @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* Shared scenario state byte of the same tuple: this overlay's progress chain
 * reads it to select its handler, and this body stores 2 into it with one `sb`.
 * @source 0x80146872 @kind unknown */
extern s8 D_80146872;
/* Primary state byte and sub-state byte of the shared state tuple, stored with
 * one `sb` each; dispatchPrimaryStateHandlerScenarioScena0400_801F863C indexes
 * its own handler table with the primary byte.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* Mode byte of the shared main-RAM state tuple, taken here with one `lbu`; the
 * two-value switch on it selects which of two helpers runs.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* Shared mode byte the front-end arms write with one `sb`.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* Route byte of the shared scenario block, stored with one `sb`.
 * @source 0x80143F1D @kind unknown */
extern u8 D_80143F1D;
/* Shared scenario progress word: three of this body's four clears go through
 * this symbol with one `sw` each. The route-0x36 clear addresses the same cell
 * as a fixed-address lvalue instead, because the original re-reads the route
 * word in that arm and a named-symbol store there lets the compiler carry the
 * earlier load across it.
 * @source 0x80146864 @kind unknown */
extern u32 g_ScenarioProgress;
/* First byte of the sound/submission argument block, passed to the shared
 * submit helper as its first argument.
 * @source 0x80145029 @kind unknown */
extern u8 D_80145029;
/* Pad/party record flags of the main-RAM record table at 0x80145E98: this body
 * clears bit 0x20 of three of them with `andi` plus one `sb` each. The first
 * clear goes through a pointer local because the original materialises that
 * cell's symbol address into a register once and reuses it for both the `lbu`
 * and the `sb`, while the other two keep the named-symbol form.
 * @source 0x80145EB4 @kind unknown */
extern u8 D_80145EB4;
/* @source 0x80145FF4 @kind unknown */
extern u8 D_80145FF4;
/* @source 0x80146134 @kind unknown */
extern u8 D_80146134;

/* Front-end record row table of the shared main-executable data segment: one
 * 0x74-byte row per claimable record, indexed here by the allocator's answer.
 * The original addresses every store with a `lui`/`addu` base plus the row
 * offset, so each cell is declared as a byte base.
 * @source 0x80143FC8 @kind table */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind table */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind table */
extern u8 D_80143FD1[];
/* @source 0x8014402C @kind table */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind table */
extern u8 D_80144030[];
/* @source 0x80144034 @kind table */
extern u8 D_80144034[];
/* Row halfword pair of the shared main-RAM data segment, read with one `lh`
 * each and widened to a word store into the claimed row.
 * @source 0x801492DA @kind unknown */
extern s16 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern s16 D_801492DC;
/* Scratchpad byte at the base of the 0x1F800000 scratch area, where the shared
 * record allocator's answer is published; the original stores it once per claim
 * and then re-reads it before every row store, and keeps its address in $s0
 * across the second allocator call.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* Shared stage wait and stage commit helpers of the main executable, outside
 * this payload: the wait answers nonzero once the stage it is asked about has
 * been armed, and the commit clears that stage.
 */
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);

/* Remaining shared helpers of the main executable, all outside this payload:
 * front-end startup, primary state arming and the 0x74-byte record allocator,
 * which answers 0xFF when every row is occupied. The sound block submission
 * pair func_80161CD0/func_80161C20 is declared by include/bof3/core.h, so it
 * takes no target-local prototype here. */
void func_8015C088(void);
void func_8015C100(void);
void func_801C1DF0(s32 arg0);
void func_801BE1B0(s32 arg0);
void func_801C601C(s32 arg0);
void func_801A4BC0(s32 arg0, s32 arg1, s32 arg2);
u8 func_8019601C(void);

/* @source 0x801F817C
 * @behavior Per-frame state-1 handler of this overlay's three-word progress
 *   chain at 0x801FA204, which dispatchProgressHandlerScenarioScena0400_801F8124
 *   selects with the signed shared scenario state byte D_80146872; the entry-0
 *   sibling advances that byte to 1 and this body closes by storing 2 into it,
 *   so the chain moves from state 1 to state 2. It gates on the shared route
 *   word D_80143F00 and, for two routes, on the sub-step byte D_80143F03; every
 *   stage test goes through the shared stage wait func_8015B5D4 with the shared
 *   scenario bank word D_8014686C as its first argument, and every committed
 *   stage through func_8015B580 with the same argument.
 *   Route 0x28 with sub-step 7, when the stage-3 wait reports zero, commits
 *   stage 3, submits the pair 0x64/8 through func_80161CD0 with D_80145029 and
 *   through func_80161C20 with 0x3F, runs the front-end startup helper
 *   func_8015C088, requests primary state 1 with sub-state 7 (D_80146874 = 1,
 *   D_80146875 = 7) and arms helpers 1 and 1 through func_801C1DF0 and
 *   func_801BE1B0. The same route then takes the stage-4 wait and, when it is
 *   armed and the stage-5 wait reports zero, selects on the shared mode byte
 *   D_80146865: mode 0 stores 0x1F into D_8014832E, and mode 1 arms through
 *   func_801C601C(1); every mode from 2 up, and mode 1 after that arming, runs
 *   func_8015C100.
 *   Route 0x2A, when the stage-1 wait reports zero, commits stage 1, clears
 *   the shared scenario progress word and disarms helpers 0 and 0 through
 *   func_801C1DF0 and func_801BE1B0; it then runs func_8015C100 and stores 0x1F
 *   into D_8014832E when the stage-3 wait is armed and the stage-4 wait reports
 *   zero.
 *   Route 0x30 with sub-step 3 clears the progress word, and when the stage-6
 *   wait reports zero commits stage 6, stores 5 into D_80143F1D and arms helper
 *   3 through func_801BE1B0; when the stage-7 wait reports zero in the same
 *   arm it submits the two records 0x83 and 0x84 with the pair 6/0x51 through
 *   func_801A4BC0.
 *   Route 0x36 only clears the progress word, and the route word is read again
 *   after that clear, so the value the route-0x41 test compares is a fresh read
 *   of the shared cell rather than the one the 0x36 test consumed.
 *   Route 0x41, when the stage-7 wait is armed and the stage-0xC wait reports
 *   zero, clears the progress word, disarms through func_801C601C(0), clears
 *   bit 0x20 of the three record flags D_80145EB4, D_80145FF4 and D_80146134,
 *   and claims up to two front-end records from the shared allocator
 *   func_8019601C. Each allocator answer is published in scratchpad byte
 *   0x1F800000 before it is tested, so a full table still leaves that byte
 *   holding 0xFF, and only an answer other than 0xFF seeds a row: occupancy 1
 *   at row byte 0 and the handler selector 0x13 (first claim) or 0x2E (second
 *   claim) at row byte 5, with the first claim also storing -0xC6 at row word
 *   0x64, the sign-extended halfwords D_801492DA and D_801492DC at row words
 *   0x68 and 0x6C and 0x40 at row byte 9 of the 0x74-byte row at 0x80143FC8.
 *   Takes no arguments, returns nothing, and the 0x20-byte frame only keeps
 *   $ra, $s1 and $s0.
 *
 * Name evidence: the two exact siblings of this target name the facts this name
 *   rests on. dispatchProgressHandlerScenarioScena0400_801F8124 indexes the
 *   chain table 0x801FA204 with D_80146872 and lists this address as its entry
 *   1, and advanceStateAndClearSharedByteScenarioScena0400_801F8160 records that
 *   entry 1 ends by advancing D_80146872 to 2; both dispatch comments and
 *   requestPrimaryState1AndFrontEndModeOnPackedRecordMatch-
 *   ScenarioScena0400_801F9DF4 (0x801F9DF4) call D_80143F00 the shared route
 *   word and func_8015B5D4/func_8015B580 the stage wait and stage commit, so the
 *   route gating, the stage commits and the closing state advance are read from
 *   those sources rather than inferred from this body's own shape.
 *
 * Shape: seeded from this span's own bytes. The three levers were classified
 *   before the first edit: the range opens on its own `addiu $sp` prologue and
 *   closes on `jr $ra`, and `analysis data-candidates` ranks nothing in this
 *   target, so the data lever is declined on measured evidence; the row starts
 *   at its own declared entry and ends at the next declared start (0x801F863C),
 *   so no fragment boundary is left to regroup and carve is ineligible; and the
 *   0x4C0-byte span has no byte-identical twin anywhere under out/binaries. A
 *   little-endian scan with every `jal` and branch word masked to its opcode
 *   leaves only the self-match at payload offset 0x157C, and a repeat at
 *   opcode/register granularity with all immediates dropped finds no member at
 *   all, so clone is ineligible and the body is written from its own bytes.
 *
 *   Five shapes were measured against the original: the frame carried one saved
 *   register too few and the switch dispatched as a compare chain (266/304 at
 *   1212 bytes); a scratchpad pointer local plus publishing the allocator
 *   answer before its test (269); a two-value switch spelled with a third case
 *   sharing the default arm (269); a pointer local for the first pad-flag clear,
 *   whose address the original materialises once for both the load and the store
 *   (281 at the original 1216 bytes); and the route-0x36 progress store written
 *   as a fixed-address lvalue (304/304 at 1216 bytes). The last one is forced by
 *   the original's own reload: the compiler's cse pass carries the route word
 *   across a named-symbol progress store, so the arm re-reads the cell with a
 *   fresh `lui`/`lhu` pair only when that store's address is one the compiler
 *   cannot prove disjoint from the route word. The three other progress stores
 *   are followed by calls, so their spelling stays through the named cell.
 *
 *   The two claimed-row arms copy the statement order and the
 *   `extern u8 D_...[]` base spelling of this repository's exact
 *   claimFrontEndRecord lifts in src/bof3/plchar, whose bodies publish the
 *   allocator answer into 0x1F800000 and index the same 0x74-byte row table at
 *   0x80143FC8; the row offset is not carried in a local here, because each of
 *   this span's eight row stores recomputes the multiply-by-0x74 chain from a
 *   fresh `lbu` of the scratchpad byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void commitStagesForSharedRouteStepsAndAdvanceScenarioStateScenarioScena0400_801F817C(
    void) {
  u8 index;
  u8 *slot;
  u8 *padFlag;

  if (D_80143F00 == 0x28) {
    if (D_80143F03 == 7) {
      if (func_8015B5D4(D_8014686C, 3) == 0) {
        func_8015B580(D_8014686C, 3);
        g_ScenarioProgress = 0;
        func_80161CD0(D_80145029, 0x64, 8);
        func_80161C20(0x3F, 0x64, 8);
        func_8015C088();
        D_80146874 = 1;
        D_80146875 = 7;
        func_801C1DF0(1);
        func_801BE1B0(1);
      }
    }
    if (func_8015B5D4(D_8014686C, 4) != 0) {
      if (func_8015B5D4(D_8014686C, 5) == 0) {
        switch (D_80146865) {
          case 0:
            D_8014832E = 0x1F;
            break;
          case 1:
            func_801C601C(1);
          case 2:
          default:
            func_8015C100();
            break;
        }
      }
    }
  }
  if (D_80143F00 == 0x2A) {
    if (func_8015B5D4(D_8014686C, 1) == 0) {
      func_8015B580(D_8014686C, 1);
      g_ScenarioProgress = 0;
      func_801C1DF0(0);
      func_801BE1B0(0);
    }
    if (func_8015B5D4(D_8014686C, 3) != 0) {
      if (func_8015B5D4(D_8014686C, 4) == 0) {
        func_8015C100();
        D_8014832E = 0x1F;
      }
    }
  }
  if (D_80143F00 == 0x30) {
    if (D_80143F03 == 3) {
      g_ScenarioProgress = 0;
      if (func_8015B5D4(D_8014686C, 6) == 0) {
        func_8015B580(D_8014686C, 6);
        D_80143F1D = 5;
        func_801BE1B0(3);
      }
      if (func_8015B5D4(D_8014686C, 7) == 0) {
        func_801A4BC0(0x83, 6, 0x51);
        func_801A4BC0(0x84, 6, 0x51);
      }
    }
  }
  if (D_80143F00 == 0x36) {
    PSX_REF(u32, 0x80146864u) = 0;
  }
  if (D_80143F00 == 0x41) {
    if (func_8015B5D4(D_8014686C, 7) != 0) {
      if (func_8015B5D4(D_8014686C, 0xC) == 0) {
        g_ScenarioProgress = 0;
        func_801C601C(0);
        padFlag = &D_80145EB4;
        *padFlag &= 0xDF;
        D_80145FF4 &= 0xDF;
        D_80146134 &= 0xDF;
        slot = D_1F800000;
        index = func_8019601C();
        *slot = index;
        if (index != 0xFFu) {
          D_80143FC8[*slot * 0x74u] = 1;
          D_80143FCD[*slot * 0x74u] = 0x13;
          *(s32 *)(D_8014402C + *slot * 0x74u) = -0xC6;
          *(s32 *)(D_80144030 + *slot * 0x74u) = D_801492DA;
          *(s32 *)(D_80144034 + *slot * 0x74u) = D_801492DC;
          D_80143FD1[*slot * 0x74u] = 0x40;
        }
        index = func_8019601C();
        *slot = index;
        if (index != 0xFFu) {
          D_80143FC8[*slot * 0x74u] = 1;
          D_80143FCD[*slot * 0x74u] = 0x2E;
        }
      }
    }
  }
  D_80146872 = 2;
}
