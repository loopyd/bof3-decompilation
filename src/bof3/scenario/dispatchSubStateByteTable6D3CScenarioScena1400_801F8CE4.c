#include "bof3/bof3.h"

/* Shared scenario progress byte the two move arms wait on: the first needs 1
 * and the second 2, and the leading arm clears it.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared progress byte of the current scene entry: the 0x0B arm waits for 0x18,
 * the 0x16 arm seeds 0x20 and the 0x18 arm waits for 0x31 before advancing it.
 * @source 0x80146867 @kind data
 */
extern u8 D_80146867;

/* Shared flag-bank word: the leading arm raises bit 0x10, the 0x0E arm raises
 * bit 0x46 of the byte bank at 0x80144F28 and the 0x18 arm raises bit 0x11,
 * each through func_8015B580.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

/* Shared primary state byte of the front-end state tuple, cleared by the two
 * reset tails.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the same tuple and the switch operand of
 * this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared tick halfword: the 0x12 arm seeds it with 0x50 and the 0x13 arm ticks
 * it down once per frame.
 * @source 0x80146876 @kind data
 */
extern u16 D_80146876;

/* Shared gate halfword: every arm that starts a front-end step returns while it
 * still reads nonzero.
 * @source 0x80143C40 @kind data
 */
extern u16 D_80143C40;

/* Shared front-end phase byte the arms test against 2 and publish 2 in.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared position halfword the two move arms hand to the overlay move helper.
 * @source 0x801492DA @kind data
 */
extern s16 D_801492DA;

/* Shared halfword the 0x12 arm halves once, as a signed 16-bit value.
 * @source 0x8014932E @kind data
 */
extern u16 D_8014932E;

/* Shared front-end step byte the arms that start a step publish 0, 0x1F.
 * @source 0x8014832E @kind data
 */
extern u8 D_8014832E;

/* Shared six-byte bank whose bit 0x46 the 0x0E arm raises.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Shared bytes 0x1D, 0x1E and 0x1F of the front-end selector words the two
 * selector-context arms publish 0xFF and 0x27 in.
 * @source 0x80143F1D @kind data
 */
extern u8 D_80143F1D;
/* @source 0x80143F1E @kind data
 */
extern u8 D_80143F1E;
/* @source 0x80143F1F @kind data
 */
extern u8 D_80143F1F;

/* Shared 0x74-byte field-record table: the object byte func_8019601C answers
 * selects the row whose occupancy byte and bytes 1 and 5 the 0x0F and 0x10 arms
 * seed.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

/* The byte-5 column of the same 0x74-byte field-record table, which the 0x0F arm
 * seeds through its own table base.
 * @source 0x80143FCD @kind table
 */
extern u8 D_80143FCD[];

/* Overlay-local object byte of this payload's trailing data word: the 0x0F arm
 * stores the object func_8019601C answers in it and the 0x10 arm indexes the
 * field-record table with it. */
extern u8 D_801FC4C0;

void func_8015B580(u32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
u8 func_801F7A18(u32 arg0, s16 arg1, s16 arg2, s32 arg3, u8 arg4, u32 arg5);
void func_801BE1B0(u32 arg0);
void func_8014ECAC(u16 arg0);
void func_80150224(s32 action_id);
u8 func_8019601C(void);
void func_80161BBC(u32 selection_id);
s32 func_80162D00(void);
void stepViewReferenceDistances(
    void);

/* @source 0x801F8CE4
 * @behavior Primary-state handler of this scenario overlay and entry 4 of the
 *           ten-word handler table at 0x801FC424 that
 *           dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C indexes with
 *           the shared primary state byte D_80146874, so a primary state
 *           reading 4 runs it. It steps the shared secondary sub-state byte
 *           D_80146875 through this overlay's front-end entry chain, selecting
 *           the arm from the 25-slot jump table the payload keeps at
 *           0x801F6D3C; the byte is read with lbu, the bound test is `sltiu`
 *           against 0x19 and the state byte and its bound test are hoisted
 *           above the frame setup. Sub-state 0 clears the progress byte, raises
 *           flag 0x10 of the bank D_8014686C through func_8015B580, seeds the
 *           selector context func_8019FA28(0x94, 0x180000, 0x268000, 0x82) and
 *           advances to 1. Sub-state 1 hands action 0x13 to the overlay move
 *           helper func_801F7A18 with the shared position halfword D_801492DA,
 *           x -0x344, step 0x284, 0x98 and 0 while the progress byte reads 1,
 *           and advances to 2 when the helper reports a nonzero low byte.
 *           Sub-state 2 repeats that hand-off with x -0x2AA, step 0x200 and 0x98
 *           at progress 2 and clears the state tuple instead when it reports
 *           one. Sub-state 0x0A hands 3 to func_801BE1B0 and advances to 0x0B,
 *           which waits for the scene progress byte D_80146867 to read 0x18,
 *           sets front-end mode 0 through func_8014ECAC and advances to 0x0C.
 *           The 0x0C arm waits for the gate halfword D_80143C40 to clear,
 *           publishes 0 in D_8014832E, starts action 0x14 through
 *           func_80150224, publishes phase 2 in D_80143BB0 and advances to 0x0D,
 *           which advances to 0x0E once the phase leaves 2. The 0x0E arm raises
 *           bit 0x46 of D_80144F28 through func_8015B580, seeds the selector
 *           context func_8019FA28(0x95, 0x1C0000, 0xC0000, 0x80), publishes 0xFF
 *           in D_80143F1E and D_80143F1D and 0x27 in D_80143F1F, and advances to
 *           0x0F, which waits on the gate, stores the object byte func_8019601C
 *           answers in D_801FC4C0 and, unless that byte is 0xFF, marks its
 *           0x74-byte row at D_80143FC8 (+0 = 1, +5 = 0x8D). Sub-state 0x0F then
 *           runs the overlay view step
 *           stepViewReferenceDistances,
 *           publishes 0x1F in D_8014832E, sets front-end mode 1 and advances to
 *           0x10; the 0x10 arm waits on the same gate, runs the view step, starts
 *           action 1, publishes phase 2 in D_80143BB0 and sets byte 1 of the
 *           D_801FC4C0 row in D_80143FC8 to 2. The 0x11, 0x12 and 0x13 arms share
 *           one tail that runs the view step: 0x11 reaches it directly, 0x12
 *           reaches it directly while the phase reads 2 and otherwise after
 *           handing action 0x13, x -0x4BA, step 0x360 and 0x78 to the move
 *           helper, on whose nonzero low byte it seeds the tick halfword
 *           D_80146876 with 0x50, advances to 0x13 and halves D_8014932E as a
 *           signed 16-bit value, and 0x13 reaches it after ticking D_80146876
 *           down and, at zero, setting front-end mode 0 and advancing to 0x14.
 *           The 0x14 arm waits on the gate, publishes 0 in D_8014832E, starts
 *           action 2, publishes phase 2 and advances to 0x15, which advances to
 *           0x16 once the phase leaves 2. The 0x16 arm seeds D_80146867 with
 *           0x20, seeds the selector context func_8019FA28(0xA7, 0x280000,
 *           0x630000, 0x8A), publishes 0xFF in all three selector bytes
 *           D_80143F1E, D_80143F1D and D_80143F1F and advances to 0x17, which
 *           waits on the gate, sets front-end mode 1, publishes 0x1F in
 *           D_8014832E, requests selection 0x8C through func_80161BBC and
 *           advances to 0x18. Sub-state 0x18 waits for D_80146867 to read 0x31
 *           and for func_80162D00 to answer nonzero, then increments that byte,
 *           raises bit 0x11 of D_8014686C through func_8015B580 and clears the
 *           state tuple. The seven slots the table leaves empty (3 through 9)
 *           return untouched. The body takes no arguments, returns nothing and
 *           uses a 0x20-byte frame holding $ra at 0x1C, the progress-byte
 *           pointer of the 0x18 arm in $s0 at 0x18 and the two outgoing argument
 *           words of the move helper at 0x10 and 0x14. The jump table is placed
 *           by this target's matching section-placement row for the .rodata
 *           section at 0x801F6D3C.
 *
 * Name evidence: the body is entry 4 of the reviewed primary-state handler table
 * at 0x801FC424, it switches on the shared secondary sub-state byte D_80146875
 * like its two in-target siblings, and 0x801F6D3C is this payload's own leading
 * table-data jump table, so the in-target family form of
 * callSubByteScenario and
 * dispatchSubStateByteTable6DA4ScenarioScena1400_801F9320 names this body
 * dispatchSubStateByteTable6D3CScenarioScena1400.
 *
 * Levers: the data lever is declined because the range opens with the
 * `lui`/`lbu` of D_80146875 and the `addiu $sp, $sp, -0x20` prologue, ends with
 * `jr $ra` and holds twenty-three `jal` sites of its own; the carve lever is declined
 * because the span is the complete declared row from 0x801F8CE4 to the next
 * declared entry 0x801F924C, with its own prologue, its own jump-table
 * dispatch and its own return path, and the neighbouring view step it calls is
 * already a separate row; the clone lever is declined because a scan of the
 * extracted payloads finds no copy of this 0x568-byte span - the only hit is
 * this target's own payload at file offset 0x20E4 - since the span names this
 * payload's own jump table at 0x801F6D3C and its own trailing object byte
 * D_801FC4C0.
 *
 * Shape: the statement order, operand forms and declaration style follow the
 * two exact in-target switch bodies, dispatchSubStateByteTable6DA4ScenarioScena1400_801F9320
 * for the switch, the per-arm return tails, the pointer-locals that keep a
 * counter address in one register and the D_80143FC8 row writes, and
 * callSubByteScenario for the
 * selector-context arms and the D_80144F28 bank. Every literal, call constant
 * and bound here is this span's own bytes.
 *
 * Representation: the 0x0F arm seeds the fifth byte of the selected record
 * through the table's byte-5 column base rather than through the row base plus
 * a constant, because the first spelling - one table base with the offsets 0
 * and 5 folded into the index expressions - made gcc reserve eight bytes of
 * stack variables and so grow the frame to 0x28; that seed measured 340/346
 * instructions, 98.27%, with the frame words of the prologue and the epilogue
 * as its only six differences. Splitting the second store onto the D_80143FCD
 * base kept both multiply chains the original carries, dropped the stack
 * variables, restored the 0x20 frame and matched at 346/346.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubStateByteTable6D3CScenarioScena1400_801F8CE4(void) {
  switch (D_80146875) {
  case 0:
    g_ScenarioProgress = 0;
    func_8015B580(D_8014686C, 0x10);
    func_8019FA28(0x94, 0x180000, 0x268000, 0x82);
    D_80146875 = 1;
    return;
  case 1:
    if (g_ScenarioProgress != 1) {
      return;
    }
    if (func_801F7A18(0x13, -0x344, D_801492DA, 0x284, 0x98, 0) == 0) {
      return;
    }
    D_80146875 = 2;
    return;
  case 2:
    if (g_ScenarioProgress != 2) {
      return;
    }
    if (func_801F7A18(0x13, -0x2AA, D_801492DA, 0x200, 0x98, 0) == 0) {
      return;
    }
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 0x0A:
    func_801BE1B0(3);
    D_80146875 = 0x0B;
    return;
  case 0x0B:
    if (D_80146867 != 0x18) {
      return;
    }
    func_8014ECAC(0);
    D_80146875 = 0x0C;
    return;
  case 0x0C:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x14);
    D_80143BB0 = 2;
    D_80146875 = 0x0D;
    return;
  case 0x0D:
    if (D_80143BB0 == 2) {
      return;
    }
    D_80146875 = 0x0E;
    return;
  case 0x0E:
    func_8015B580((u32)&D_80144F28, 0x46);
    func_8019FA28(0x95, 0x1C0000, 0xC0000, 0x80);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0x27;
    D_80146875 = 0x0F;
    return;
  case 0x0F:
    if (D_80143C40 != 0) {
      return;
    }
    D_801FC4C0 = func_8019601C();
    if (D_801FC4C0 != 0xFF) {
      D_80143FC8[(u32)D_801FC4C0 * 0x74U] = 1;
      D_80143FCD[(u32)D_801FC4C0 * 0x74U] = 0x8D;
    }
    stepViewReferenceDistances();
    D_8014832E = 0x1F;
    func_8014ECAC(1);
    D_80146875 = 0x10;
    return;
  case 0x10:
    if (D_80143C40 != 0) {
      return;
    }
    stepViewReferenceDistances();
    func_80150224(1);
    D_80143BB0 = 2;
    D_80143FC8[(u32)D_801FC4C0 * 0x74U + 1] = 2;
    D_80146875 = 0x11;
    return;
  case 0x11:
    stepViewReferenceDistances();
    return;
  case 0x12:
    if (D_80143BB0 != 2) {
      if (func_801F7A18(0x13, -0x4BA, D_801492DA, 0x360, 0x78, 0) != 0) {
        D_80146876 = 0x50;
        D_80146875 = 0x13;
        D_8014932E = (s16)D_8014932E / 2;
      }
    }
    stepViewReferenceDistances();
    return;
  case 0x13: {
    u16* counter = &D_80146876;

    if (--*counter == 0) {
      func_8014ECAC(0);
      D_80146875 = 0x14;
    }
    stepViewReferenceDistances();
    return;
  }
  case 0x14:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(2);
    D_80143BB0 = 2;
    D_80146875 = 0x15;
    return;
  case 0x15:
    if (D_80143BB0 == 2) {
      return;
    }
    D_80146875 = 0x16;
    return;
  case 0x16:
    D_80146867 = 0x20;
    func_8019FA28(0xA7, 0x280000, 0x630000, 0x8A);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x17;
    return;
  case 0x17:
    if (D_80143C40 != 0) {
      return;
    }
    func_8014ECAC(1);
    D_8014832E = 0x1F;
    func_80161BBC(0x8C);
    D_80146875 = 0x18;
    return;
  case 0x18: {
    u8* progress = &D_80146867;

    if (*progress != 0x31) {
      return;
    }
    if (func_80162D00() == 0) {
      return;
    }
    (*progress)++;
    func_8015B580(D_8014686C, 0x11);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  }
  }
}
