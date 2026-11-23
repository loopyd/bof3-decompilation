#include "bof3/bof3.h"

/* Shared scenario progress byte the progress-gated arms compare against 1, 2,
 * 0x1C and 0x1E, that two arms clear and that two others increment.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte: the two arms that only hand work to the front
 * end return while it still reads 2, and the two action arms publish 2 in it.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary state byte of the front-end state tuple, cleared by the three
 * reset tails.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the same tuple and the switch operand of
 * this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared tick halfword: the three countdown arms decrement it once per frame,
 * one arm seeds it with 0x78, 0x10 or 0x64, and one arm tests it.
 * @source 0x80146876 @kind data
 */
extern u16 D_80146876;

/* Shared flag-bank word the seeding arms pass to the bit helpers.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

/* Shared world-flag halfword whose bit 3 the release arm clears.
 * @source 0x80146258 @kind data
 */
extern u16 D_80146258;

/* Shared record flag bytes the 0x0B arm raises.
 * @source 0x80146158 @kind data
 */
extern u8 D_80146158;
/* @source 0x80146018 @kind data
 */
extern u8 D_80146018;
/* @source 0x80145ED8 @kind data
 */
extern u8 D_80145ED8;

/* Shared selection byte the two arms hand to the front-end record helpers.
 * @source 0x80145029 @kind data
 */
extern u8 D_80145029;

/* Shared position halfwords the two move arms hand to the overlay move helper.
 * @source 0x801492DA @kind data
 */
extern s16 D_801492DA;

/* Shared halfword counter the two scroll arms step by 0x58 in the two
 * directions of the move.
 * @source 0x8014932C @kind data
 */
extern u16 D_8014932C;

/* Shared status byte the same two arms publish mode 2 in.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared 0x74-byte field-record table: the object byte func_8019601C answers
 * selects the row whose occupancy byte and byte 5 the 0x0C arm seeds.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

u8 func_801F7A18(u32 arg0, s16 arg1, s16 arg2, s32 arg3, u8 arg4, u32 arg5);
u8 func_8019601C(void);
void func_801C601C(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
void func_8015C058(void);
void func_80150224(s32 action_id);
void func_801BE1B0(u32 arg0);
void func_80161D20(s32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
void func_80166180(s32 arg0);
u32 func_80163EA0(void);
void func_80161D74(s32 arg0);
void signedTableStepScaled(
    u8 amount);

/* @source 0x801F9320
 * @behavior Primary-state handler of this scenario overlay and entry 5 of the
 *           ten-word handler table at 0x801FC424 that
 *           dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C indexes with
 *           the shared primary state byte D_80146874, so a primary state
 *           reading 5 runs it. It steps the shared secondary sub-state byte
 *           D_80146875 through this overlay's front-end entry chain, selecting
 *           the arm from the 34-slot jump table the payload keeps at
 *           0x801F6DA4; the byte is read with lbu, the bound test is `sltiu`
 *           against 0x22 and the state byte and its bound test are hoisted
 *           above the frame setup. Sub-states 0 and 1 hand action 0x13 to the
 *           overlay move helper func_801F7A18 with the shared position halfword
 *           D_801492DA as its second coordinate, the x -0x318 or -0x2AA, the
 *           step 0x150 or 0x200, 0x70 or 0x40 and 0 while the scenario progress
 *           byte reads 1 or 2, and advance to 1 or 2 when it reports a nonzero
 *           low byte. Sub-state 5 queues front-end action 1 through
 *           func_801C601C and advances to 6. Sub-state 6, at progress 1, sets
 *           flag 0xE of the bank D_8014686C through func_8015B580, seeds the
 *           tick halfword with 0x78 and advances to 7. Sub-state 7 ticks the
 *           halfword down and, at zero, requests primary progress 2 and
 *           sub-state 2. Sub-state 0x0A, at progress 0x1C, hands the three
 *           constants 0xB8000, 0x728000 and 0x25 to func_801BFE34 and advances
 *           to 0x0B, which at progress 0x1E raises the record flag bytes
 *           D_80146158, D_80146018 and D_80145ED8, seeds the tick halfword with
 *           0x10 and advances to 0x0C. Sub-states 0x0C and 0x0E tick the
 *           halfword down; while it still reads nonzero they publish mode 2 in
 *           D_80149333 and step the scroll halfword D_8014932C by 0x58 in the
 *           direction of the move, and at zero the 0x0C arm asks func_8019601C
 *           for an object byte, seeds that object's 0x74-byte row at
 *           D_80143FC8 (+0 = 1, +5 = 0x8E) unless the byte is 0xFF, reseeds the
 *           tick halfword with 0x64 and advances to 0x0D, while the 0x0E arm
 *           hands 0x32 to func_801C7A54 and advances to 0x0F. Sub-states 0x0C,
 *           0x0D and 0x0F then run the overlay step handler
 *           signedTableStepScaled
 *           with the per-tick scale 4. Sub-state 0x10 runs the shared front-end
 *           step func_8015C058, clears bit 3 of D_80146258, clears the progress
 *           byte, sets flag 0xF of D_8014686C through func_8015B580 and then
 *           clears the state tuple. Sub-state 0x14 starts action 0x1C through
 *           func_80150224, publishes phase 2 and advances to 0x15, which clears
 *           the state tuple through func_8015C058 while the phase is not 2.
 *           Sub-state 0x1E clears the progress byte, hands 3 to func_801BE1B0
 *           and advances to 0x1F, which at progress 1 hands the selection byte
 *           D_80145029 to func_80161D20, runs func_801636A0(2, 1) and
 *           func_80166180(0x0D), starts action 0x1D through func_80150224,
 *           publishes phase 2 and advances to 0x20. Sub-state 0x20 returns
 *           while the low byte func_80163EA0 answers reads 0 or while the phase
 *           still reads 2, hands D_80145029 to func_80161D74, advances to 0x21
 *           and increments the progress byte. Sub-states 2 and 0x21 share one
 *           arm: while the progress byte reads 0 it runs func_8015C058 and
 *           clears the state tuple. The remaining slots of the table return
 *           untouched. It takes no arguments and returns nothing; its 0x20-byte
 *           frame keeps $ra at 0x1C and, for the 0x0C arm's tick pointer, $s0 at
 *           0x18, and the two outgoing argument words of the move helper sit at
 *           0x10 and 0x14.
 *
 * Name evidence: the body is entry 5 of the reviewed primary-state handler
 * table at 0x801FC424, the byte it switches on is the shared secondary
 * sub-state byte D_80146875 that this target's other sources name as the
 * second member of the front-end state tuple, and 0x801F6DA4 is this payload's
 * own leading table-data jump table, so the in-target family form of
 * callSubByteScenario names this body
 * dispatchSubStateByteTable6DA4ScenarioScena1400.
 *
 * Shape: the payload holds no byte-identical copy of this 0x514-byte span and
 * no neighbour either - a scan of the 398 extracted payloads finds no aligned
 * span sharing more than the opening prologue words, and the span names this
 * payload's own jump table at 0x801F6DA4 and its own table-data word, which no
 * other payload carries, so no clone or carver rung applies. The statement
 * order and declaration style come from the two exact in-target sources that
 * this body's own bytes confirm site by site:
 * callSubByteScenario for the switch, the
 * per-arm return tails and the declarations of D_80146875, D_80143BB0 and the
 * shared helpers, and claimFrontEndRecordHandler34FromPositionPairStoreStateDistancePlus200AndSetRowByteBPlcharPlp46700_801CF050
 * for the func_8019601C object-byte block whose 0x74 multiply chain, 0xFF test
 * and `row`/`index` locals this arm mirrors. Every literal, call constant and
 * bound here is this span's own.
 *
 * Representation: every countdown arm reads the tick halfword through its own
 * pointer local, which is what keeps that address in one register for the load
 * and the store, and the 0x0C arm has the only such pointer that survives a
 * call, so it is the one that lands in $s0. The two arms that increment the
 * progress byte do the same through a pointer local, which leaves its address
 * in one register for the load and the delay-slot store instead of
 * re-materialising it; that one local is worth one instruction per arm. The
 * shared tails are written once per arm - the step handler call in the 0x0C and
 * 0x0D arms, the front-end step and state-tuple clear in the 0x10, 0x15, 2 and
 * 0x21 arms - which is what the original's single cross-jumped copies at
 * 0x801F9620 and 0x801F9808 are. Losing spellings measured on the way here: the
 * plain `--D_80146876` decrements with the field-record stores through two
 * separate table indexes scored 241/328 instructions with a 1312-byte body, and
 * the same body with the per-case tick pointers but direct `g_ScenarioProgress++`
 * increments scored 282/326 with a 1304-byte body; the present shape measured
 * 323/325 with the 1300-byte body, the two remaining instructions being the
 * jump-table load words that the placement row covers. Those probes ran while
 * the section-placement row was withheld so the body stayed measurable; it was
 * restored before the gate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubStateByteTable6DA4ScenarioScena1400_801F9320(void) {
  u8* rec;
  u8 index;

  switch (D_80146875) {
  case 0:
    if (g_ScenarioProgress != 1) {
      return;
    }
    if (func_801F7A18(0x13, -0x318, D_801492DA, 0x150, 0x70, 0) == 0) {
      return;
    }
    D_80146875 = 1;
    return;
  case 1:
    if (g_ScenarioProgress != 2) {
      return;
    }
    if (func_801F7A18(0x13, -0x2AA, D_801492DA, 0x200, 0x40, 0) == 0) {
      return;
    }
    D_80146875 = 2;
    return;
  case 2:
    if (g_ScenarioProgress == 0) {
      func_8015C058();
      D_80146874 = 0;
      D_80146875 = 0;
    }
    return;
  case 5:
    if (D_80143BB0 == 2) {
      return;
    }
    func_801C601C(1);
    D_80146875 = 6;
    return;
  case 6:
    if (g_ScenarioProgress != 1) {
      return;
    }
    func_8015B580(D_8014686C, 0xE);
    D_80146876 = 0x78;
    D_80146875 = 7;
    return;
  case 7: {
    u16* counter = &D_80146876;

    if (--*counter != 0) {
      return;
    }
    g_ScenarioProgress = 2;
    D_80146875 = 2;
    return;
  }
  case 0x0A:
    if (g_ScenarioProgress != 0x1C) {
      return;
    }
    func_801BFE34(0xB8000, 0x728000, 0x25);
    D_80146875 = 0x0B;
    return;
  case 0x0B:
    if (g_ScenarioProgress != 0x1E) {
      return;
    }
    D_80146158 = 1;
    D_80146018 = 1;
    D_80145ED8 = 1;
    D_80146876 = 0x10;
    D_80146875 = 0x0C;
    return;
  case 0x0C: {
    u16* counter = &D_80146876;

    if (--*counter != 0) {
      D_80149333 = 2;
      D_8014932C += 0x58;
    } else {
      index = func_8019601C();
      if (index != 0xFF) {
        rec = &D_80143FC8[(u32)index * 0x74u];
        rec[0] = 1;
        rec[5] = 0x8E;
      }
      *counter = 0x64;
      D_80146875 = 0x0D;
    }
    signedTableStepScaled(
        4);
    return;
  }
  case 0x0D: {
    u16* counter = &D_80146876;
    u8* progress;

    if (--*counter == 0) {
      *counter = 0x10;
      D_80146875 = 0x0E;
      progress = &g_ScenarioProgress;
      (*progress)++;
    }
    signedTableStepScaled(
        4);
    return;
  }
  case 0x0E: {
    u16* counter = &D_80146876;

    if (--*counter != 0) {
      D_80149333 = 2;
      D_8014932C -= 0x58;
      return;
    }
    func_801C7A54(0x32);
    D_80146875 = 0x0F;
    return;
  }
  case 0x10:
    func_8015C058();
    D_80146258 &= 0xFFF7;
    g_ScenarioProgress = 0;
    func_8015B580(D_8014686C, 0xF);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 0x14:
    func_80150224(0x1C);
    D_80143BB0 = 2;
    D_80146875 = 0x15;
    return;
  case 0x15:
    if (D_80143BB0 == 2) {
      return;
    }
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 0x1E:
    g_ScenarioProgress = 0;
    func_801BE1B0(3);
    D_80146875 = 0x1F;
    return;
  case 0x1F:
    if (g_ScenarioProgress != 1) {
      return;
    }
    func_80161D20(D_80145029);
    func_801636A0(2, 1);
    func_80166180(0x0D);
    func_80150224(0x1D);
    D_80143BB0 = 2;
    D_80146875 = 0x20;
    return;
  case 0x20: {
    u8* progress;

    if ((u8)func_80163EA0() == 0) {
      return;
    }
    if (D_80143BB0 == 2) {
      return;
    }
    func_80161D74(D_80145029);
    D_80146875 = 0x21;
    progress = &g_ScenarioProgress;
    (*progress)++;
    return;
  }
  case 0x21:
    if (g_ScenarioProgress == 0) {
      func_8015C058();
      D_80146874 = 0;
      D_80146875 = 0;
    }
    return;
  }
}
