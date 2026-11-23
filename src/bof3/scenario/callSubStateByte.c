#include "bof3/bof3.h"

/* Shared scenario progress byte every gated arm of this body waits on, that two
 * arms increment and one clears.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared primary state byte of the front-end state tuple, cleared by the three
 * reset tails and set to 3 by the last arm.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the same tuple and the switch operand of
 * this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared tick halfword the countdown arms decrement and four arms seed.
 * @source 0x80146876 @kind data
 */
extern u16 D_80146876;

/* Shared flag-bank word the seeding arms hand to the bit helpers.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

/* Shared front state byte whose bit 0 the two ends of the chain touch.
 * @source 0x80146871 @kind data
 */
extern u8 D_80146871;

/* Shared front-end phase byte: the arms that only hand work to the front end
 * return while it still reads 2, and the action arms publish 2 in it.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared gate halfword four arms wait on before they start their action.
 * @source 0x80143C40 @kind data
 */
extern u16 D_80143C40;

/* Shared status byte the two ramp arms publish mode 2 in.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared scroll halfword the two ramp arms derive from the tick counter. */
extern u16 D_8014932C;

/* Shared mode byte the two progress-gated arms publish 1, 2 and 3 in.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* Shared record flag bytes the first arm and the tick-reset arm raise.
 * @source 0x80146158 @kind data
 */
extern u8 D_80146158;
/* @source 0x80146018 @kind data
 */
extern u8 D_80146018;
/* @source 0x80145ED8 @kind data
 */
extern u8 D_80145ED8;

/* Shared selector byte the flag-seeding arm publishes 2 in.
 * @source 0x8014504B @kind data
 */
extern u8 D_8014504B;

/* Shared halfword the same arm clears, stored with one `sh`. */
extern u16 D_80144F54;

/* Shared selector bytes the context arms publish 0xFF in and one arm clears.
 * @source 0x80143F1D @kind data
 */
extern u8 D_80143F1D;
/* @source 0x80143F1E @kind data
 */
extern u8 D_80143F1E;

/* Shared step counter the last two arms tick and one increments.
 * @source 0x80145E99 @kind data
 */
extern u8 D_80145E99;

/* Shared progress byte the 25 arm waits to read 0.
 * @source 0x80145FBB @kind data
 */
extern u8 D_80145FBB;

/* Shared mode byte the action arms publish before they seed the front end.
 * @source 0x8014832E @kind data
 */
extern u8 D_8014832E;

/* Shared index byte that selects the 0xA4-byte row the 17 arm seeds.
 * @source 0x80181B18 @kind data
 */
extern u8 D_80181B18;

/* Leading halfword of the 0xA4-byte-stride row the 17 arm marks. */
extern u8 D_8014497C[];

/* Byte 6 of the same 0xA4-byte-stride row, set to 9 by the 17 arm.
 * @source 0x80144982 @kind table
 */
extern u8 D_80144982[];

/* Shared 0x74-byte field-record table: the object byte func_8019601C answers
 * selects the row the 30 arm marks at +0 and +5.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind table
 */
extern u8 D_80143FCD[];

/* Overlay-local selector byte func_8019601C answers into, in the payload's
 * trailing data word at 0x801FC4C0. */
extern u8 D_801FC4C0;

void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_80150224(s32 action_id);
void func_801BE1B0(u32 arg0);
void func_8014ECAC(u16 arg0);
void func_8015DF18(u16 cue);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
u8 func_8019601C(void);
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
void signedTableStepScaled(
    u8 amount);

/* @source 0x801F7FB8
 * @behavior Primary-state handler of this scenario overlay and entry 2 of the
 *           ten-word handler table at 0x801FC424 that
 *           dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C indexes with
 *           the shared primary state byte D_80146874, so a primary state
 *           reading 2 runs it; the table word at 0x801FC42C is its only image
 *           word and no in-image jal names 0x801F7FB8. It steps the shared
 *           secondary sub-state byte D_80146875 through the overlay's
 *           front-end entry chain, selecting the arm from the 37-slot jump
 *           table the payload keeps at 0x801F6C6C; the byte is read with lbu,
 *           the bound test is `sltiu` against 0x25 and the state byte and its
 *           bound test are hoisted above the frame setup. Sub-state 0, once the
 *           progress byte reads 2, raises the record flag bytes D_80146158,
 *           D_80146018 and D_80145ED8, seeds the tick halfword with 0xF0 and
 *           advances to 1. Sub-state 1 ticks the halfword down; over the ramp
 *           it publishes 1900 * (0xF0 - tick) / 240 in D_8014932C, and at zero
 *           it pins D_8014932C at 1900, hands 0x248000, 0x168000 and 0x25 to
 *           func_801BFE34, raises the same three record flags and advances to
 *           2; both branches then publish mode 2 in D_80149333. Sub-states 2, 4
 *           and 6 test the progress byte against 4, 6 and 8: at 4 they publish
 *           1 in D_8014933B, queue cue 0x202 through func_8015DF18, seed the
 *           tick halfword with 0x1E and advance to 3; at 6 they queue the same
 *           cue, seed 0x3C and advance to 5; at 8 they publish 3 in D_8014933B,
 *           queue cue 0x200, seed 0xD2 and advance to 7. Sub-states 3 and 5 are
 *           the countdown ends of those steps: while the tick halfword still
 *           reads nonzero they tick it and run the overlay step handler
 *           signedTableStepScaled
 *           with the per-tick scale 4, and at zero the 3 arm advances to 4 and
 *           increments the progress byte while the 5 arm publishes 2 in
 *           D_8014933B, advances to 6 and increments the progress byte.
 *           Sub-state 7 ticks the halfword down, runs the same step handler,
 *           publishes 1900 * tick / 180 in D_8014932C while the tick is at most
 *           0xB4 and mode 2 in D_80149333 afterwards, and at zero clears
 *           D_8014932C, advances to 8, increments the progress byte and still
 *           publishes mode 2. Sub-state 8 queues cue 0x201 through
 *           func_8015DF18, hands 0x188000, 0x168000 and 0x31 to func_801BFE34,
 *           hands 0x31 to func_801C7A54 and advances to 9. Sub-state 0x0A
 *           requests the selector context func_8019FA28(0xBF, 0x1E8000,
 *           0x228000, 1), sets flag 4 and flag 5 of the bank D_8014686C through
 *           func_8015B580 and advances to 0x0B, which advances to 0x0C once the
 *           gate halfword D_80143C40 is clear. Sub-state 0x0C advances to 0x0D
 *           once the progress byte reads 1, through func_8014ECAC(0). Sub-state
 *           0x0D publishes 0 in D_8014832E, starts action 0x70 through
 *           func_80150224, publishes phase 2 in D_80143BB0 and advances to
 *           0x0E, which advances to 0x0F once the phase leaves 2. Sub-state 0x0F
 *           requests the selector context func_8019FA28(0xBF, 0x490000,
 *           0xD0000, 0x81), publishes 0xFF in D_80143F1E and D_80143F1D and
 *           advances to 0x10, which waits on the gate halfword, publishes 0x1F
 *           in D_8014832E, sets front-end mode 1 through func_8014ECAC and
 *           advances to 0x11. Sub-state 0x11, while the progress byte reads 0,
 *           marks the 0xA4-byte row that the payload byte D_80181B18 selects
 *           (+0 = 1, +6 = 9) of the table at D_8014497C, runs func_8015C058,
 *           sets flag 6 of D_8014686C through func_8015B580 and clears the state
 *           tuple. Sub-state 0x14 starts action 0x7D through func_80150224,
 *           publishes phase 2 and advances to 0x15, which runs func_8015C058 and
 *           clears the state tuple once the phase leaves 2. Sub-state 0x19, once
 *           D_80145FBB reads 0, runs func_8015C058, hands 3 to func_801BE1B0,
 *           sets flag 8 of D_8014686C, hands 0, 0x5A, 1 and 0 to
 *           func_801650B4 and clears the state tuple. Sub-state 0x1D runs
 *           func_8015C058, sets flag 9 of D_8014686C, hands 4 to
 *           func_801BE1B0 and clears the state tuple. Sub-state 0x1E, at
 *           progress 5, stores the object byte func_8019601C answers in
 *           D_801FC4C0 and, unless that byte is 0xFF, marks its 0x74-byte row at
 *           D_80143FC8 (+0 = 1, +5 = 0x91), then advances to 0x1F. Sub-state
 *           0x20 requests the selector context func_8019FA28(0xBD, 0x18000000,
 *           0xC000000, 2), publishes 2 in D_8014504B, clears D_80144F54, sets
 *           bit 0 of D_80146871 and advances to 0x21, which advances to 0x22
 *           once the gate halfword is clear. Sub-state 0x22 ticks the tick
 *           halfword down and, at zero, starts action 4 through
 *           func_80150224, publishes phase 2 and advances to 0x23; it then
 *           increments D_80145E99, as does sub-state 0x23, which advances to
 *           0x24 once the phase leaves 2. Sub-state 0x24 requests the selector
 *           context func_8019FA28(0x96, 0x1C8000, 0x148000, 5), clears
 *           D_80143F1E, requests primary state 3 by storing 3 in D_80146874,
 *           clears the secondary sub-state and clears bit 0 of D_80146871. The
 *           remaining slots of the table return untouched. The body takes no
 *           arguments and returns nothing; its 0x18-byte frame keeps $ra at
 *           0x14 and $s0 at 0x10 for the tick pointer the 7 arm holds across the
 *           step-handler call. The jump table is placed by this target's
 *           matching section-placement row for the .rodata section at
 *           0x801F6C6C.
 *
 * Name evidence: the body is entry 2 of the reviewed primary-state handler
 * table at 0x801FC424, it switches on the shared secondary sub-state byte
 * D_80146875 exactly as the three other handlers of this overlay do, and
 * 0x801F6C6C is the table-data base it dispatches through, so the family form
 * of callSubByte names this body
 * dispatchSubStateByteTable6C6CScenarioScena1400.
 *
 * Shape: the payload holds no byte-identical copy of this 0x848-byte span - a
 * scan of the 396 extracted payloads finds none, and the span names the
 * payload-local byte D_801FC4C0 and this payload's own jump table, which no
 * other payload carries. The nearest siblings are the three lifted handlers of
 * this same overlay (0x801F7BA0, 0x801F8CE4 and 0x801F9320), which supplied the
 * statement order, operand forms and declaration style of the shared helpers
 * while every literal here comes from this span's own bytes.
 *
 * Representation: every read-modify-write of a shared cell here goes through a
 * pointer local, which is what makes gcc materialise the cell address once
 * (`lui`/`addiu` plus `0($reg)` accesses) instead of addressing it absolutely at
 * each use, and the two countdown arms that increment the shared progress byte
 * take their pointer in the same statement position after their test so that
 * their identical tails cross-jump into the single copy the original keeps.
 * Losing spellings measured on the way here, all with the .rodata placement row
 * withheld so the body stayed measurable: addressing every shared cell directly
 * scored 428/530 instructions, the same with the 7 arm's bound written as a
 * single `< 0xB4` scored 428/530 and with `*counter != 0xB4 && *counter < 0xB4`
 * scored 429/530, pointer locals with only the 0x22 arm's increment moved after
 * its test scored 472/537, and the same with the 0x23 arm mirrored scored
 * 527/530; writing the 7 arm's tick as `(*counter)--` instead of
 * `*counter = *counter - 1` scored 528/530 by letting the decrement fill the
 * branch delay slot ahead of the step-handler argument, as the original does.
 * The bound test needs the doubled form because the original keeps the equality
 * test and the `sltiu` bound side by side.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callSubStateByte(void) {
  switch (D_80146875) {
  case 0:
    if (g_ScenarioProgress != 2) {
      return;
    }
    D_80146158 = 1;
    D_80146018 = 1;
    D_80145ED8 = 1;
    D_80146876 = 0xF0;
    D_80146875 = 1;
    return;
  case 1: {
    u16* counter = &D_80146876;

    *counter = *counter - 1;
    if (*counter != 0) {
      D_8014932C = (0xF0 - *counter) * 1900 / 240;
    } else {
      D_8014932C = 1900;
      func_801BFE34(0x248000, 0x168000, 0x25);
      D_80146158 = 1;
      D_80146018 = 1;
      D_80145ED8 = 1;
      D_80146875 = 2;
    }
    D_80149333 = 2;
    return;
  }
  case 2:
    if (g_ScenarioProgress != 4) {
      return;
    }
    D_8014933B = 1;
    func_8015DF18(0x202);
    D_80146876 = 0x1E;
    D_80146875 = 3;
    return;
  case 3: {
    u16* counter = &D_80146876;
    u8* progress;

    if (*counter != 0) {
      *counter = *counter - 1;
      signedTableStepScaled(
          4);
      return;
    }
    progress = &g_ScenarioProgress;
    D_80146875 = 4;
    (*progress)++;
    return;
  }
  case 4:
    if (g_ScenarioProgress != 6) {
      return;
    }
    func_8015DF18(0x202);
    D_80146876 = 0x3C;
    D_80146875 = 5;
    return;
  case 5: {
    u16* counter = &D_80146876;
    u8* progress;

    if (*counter != 0) {
      *counter = *counter - 1;
      signedTableStepScaled(
          4);
      return;
    }
    D_8014933B = 2;
    progress = &g_ScenarioProgress;
    D_80146875 = 6;
    (*progress)++;
    return;
  }
  case 6:
    if (g_ScenarioProgress != 8) {
      return;
    }
    D_8014933B = 3;
    func_8015DF18(0x200);
    D_80146876 = 0xD2;
    D_80146875 = 7;
    return;
  case 7: {
    u16* counter = &D_80146876;
    u8* progress;

    if (*counter != 0) {
      (*counter)--;
      signedTableStepScaled(
          4);
      if (*counter != 0xB4 && *counter < 0xB4) {
        D_8014932C = *counter * 1900 / 180;
      }
    } else {
      progress = &g_ScenarioProgress;
      D_8014932C = 0;
      D_80146875 = 8;
      (*progress)++;
    }
    D_80149333 = 2;
    return;
  }
  case 8:
    func_8015DF18(0x201);
    func_801BFE34(0x188000, 0x168000, 0x31);
    func_801C7A54(0x31);
    D_80146875 = 9;
    return;
  case 0x0A:
    func_8019FA28(0xBF, 0x1E8000, 0x228000, 1);
    func_8015B580(D_8014686C, 4);
    func_8015B580(D_8014686C, 5);
    D_80146875 = 0x0B;
    return;
  case 0x0B:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146875 = 0x0C;
    return;
  case 0x0C:
    if (g_ScenarioProgress != 1) {
      return;
    }
    func_8014ECAC(0);
    D_80146875 = 0x0D;
    return;
  case 0x0D:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x70);
    D_80143BB0 = 2;
    D_80146875 = 0x0E;
    return;
  case 0x0E:
    if (D_80143BB0 == 2) {
      return;
    }
    D_80146875 = 0x0F;
    return;
  case 0x0F:
    func_8019FA28(0xBF, 0x490000, 0xD0000, 0x81);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80146875 = 0x10;
    return;
  case 0x10:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0x1F;
    func_8014ECAC(1);
    D_80146875 = 0x11;
    return;
  case 0x11: {
    u8* index;

    if (g_ScenarioProgress != 0) {
      return;
    }
    index = &D_80181B18;
    D_80144982[(u32)*index * 0xA4u] = 9;
    *(u16*)(D_8014497C + (u32)*index * 0xA4u) = 1;
    func_8015C058();
    func_8015B580(D_8014686C, 6);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  }
  case 0x14:
    func_80150224(0x7D);
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
  case 0x19:
    if (D_80145FBB != 0) {
      return;
    }
    func_8015C058();
    func_801BE1B0(3);
    func_8015B580(D_8014686C, 8);
    func_801650B4(0, 0x5A, 1, 0);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 0x1D:
    func_8015C058();
    func_8015B580(D_8014686C, 9);
    func_801BE1B0(4);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 0x1E:
    if (g_ScenarioProgress != 5) {
      return;
    }
    D_801FC4C0 = func_8019601C();
    if (D_801FC4C0 != 0xFF) {
      D_80143FC8[(u32)D_801FC4C0 * 0x74u] = 1;
      D_80143FCD[(u32)D_801FC4C0 * 0x74u] = 0x91;
    }
    D_80146875 = 0x1F;
    return;
  case 0x20: {
    u8* front;

    func_8019FA28(0xBD, 0x18000000, 0xC000000, 2);
    front = &D_80146871;
    D_8014504B = 2;
    D_80144F54 = 0;
    D_80146875 = 0x21;
    *front |= 1;
    return;
  }
  case 0x21:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146875 = 0x22;
    return;
  case 0x22: {
    u16* counter = &D_80146876;
    u8* steps;

    *counter = *counter - 1;
    if (*counter == 0) {
      func_80150224(4);
      D_80143BB0 = 2;
      D_80146875 = 0x23;
    }
    steps = &D_80145E99;
    (*steps)++;
    return;
  }
  case 0x23: {
    u8* steps;

    if (D_80143BB0 != 2) {
      D_80146875 = 0x24;
    }
    steps = &D_80145E99;
    (*steps)++;
    return;
  }
  case 0x24: {
    u8* front;

    func_8019FA28(0x96, 0x1C8000, 0x148000, 5);
    front = &D_80146871;
    D_80143F1E = 0;
    D_80146874 = 3;
    D_80146875 = 0;
    *front &= 0xFE;
    return;
  }
  }
}
