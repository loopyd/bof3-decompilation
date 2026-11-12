#include "bof3/bof3.h"

/* Shared scenario progress byte: the first arm clears it and the second arm
 * waits for it to read 1.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte: every arm that does work tests it against 2
 * first and leaves the frame untouched while it still reads 2.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary state byte of the front-end state tuple, cleared by the two
 * reset arms.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the same tuple and the switch operand of
 * this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared front state byte whose bit 0 the two reset arms clear.
 * @source 0x80146871 @kind data
 */
extern u8 D_80146871;

/* Shared flag-bank pointer the legs pass to the bit helpers.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

/* Shared flag bank the mode-20 arm and the shared tail address directly.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Shared gate halfword the mode-2 arm waits on before it starts its action.
 * @source 0x80143C40 @kind data
 */
extern u16 D_80143C40;

/* Shared position halfwords the mode-3 and mode-5 arms hand to the overlay
 * move helper.
 * @source 0x801492D8 @kind data
 */
extern s16 D_801492D8;
/* @source 0x801492DA @kind data
 */
extern s16 D_801492DA;

/* Shared bytes the mode-1 arm copies and publishes, and the two flag bytes the
 * mode-20 tail sets.
 * @source 0x80145029 @kind data
 */
extern u8 D_80145029;
/* @source 0x8014504A @kind data
 */
extern u8 D_8014504A;
/* @source 0x8014504B @kind data
 */
extern u8 D_8014504B;
/* @source 0x80143F1D @kind data
 */
extern u8 D_80143F1D;
/* @source 0x80143F1E @kind data
 */
extern u8 D_80143F1E;
/* @source 0x80143F1F @kind data
 */
extern u8 D_80143F1F;

/* Shared halfword the mode-1 arm clears, stored with one `sh`. */
extern u16 D_80144F54;

/* Shared field-record table the mode-4 and mode-6 arms index with the payload's
 * own selector byte; each record's leading byte carries the busy bit 0.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

/* Overlay-local selector byte the move helper func_801F7A18 publishes and this
 * body indexes the field-record table with; it lives in the payload's trailing
 * data word at 0x801FC4C0. */
extern u8 D_801FC4C0;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_80150224(s32 action_id);
void func_8015C058(void);
void func_801BE1B0(u32 arg0);
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void func_801F7A18(u32 arg0, s16 arg1, s16 arg2, s32 arg3, u8 arg4, u32 arg5);

/* @source 0x801F7BA0
 * @behavior Primary-state handler of this scenario overlay: the dispatcher
 *           dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C runs it as
 *           entry 1 of the ten-word handler table at 0x801FC424, and it steps
 *           the shared secondary sub-state byte D_80146875 through this
 *           overlay's front-end entry chain, selecting the arm from the
 *           26-slot jump table the payload keeps at 0x801F6C04. Every arm that
 *           does work first returns while the shared phase byte D_80143BB0
 *           still reads 2, so the chain waits for the front end to finish the
 *           request the previous arm made. Sub-state 0 clears the scenario
 *           progress byte and queues helper action 1 through func_801BE1B0
 *           before advancing to 1. Sub-state 1, once progress reads 1, sets
 *           flag bit 1 of the bank D_8014686C through func_8015B580, requests
 *           the selector context func_8019FA28(0xBD, 0x10000000, 0x14000000,
 *           2), publishes 2 in D_8014504B, clears the paired halfword
 *           D_80144F54, copies D_80145029 into D_80143F1F and sets bit 0 of the
 *           shared front state byte D_80146871, then advances to 2. Sub-state 2
 *           starts action 0x12 through func_80150224 once the gate halfword
 *           D_80143C40 is clear, publishes phase 2 and advances to 3.
 *           Sub-states 3 and 5, while the phase is not 2, call the overlay move
 *           helper func_801F7A18 with action 0x13, the shared position pair
 *           D_801492D8/D_801492DA, the step -0x200 or 0x400, 0x20 and 0, and
 *           advance to 4 and 6; while the phase still reads 2 they hand 0x98
 *           and 0x28 or 0x38 to func_801647C4 instead. Sub-states 4 and 6 start
 *           action 0x13 or 0x14 through func_80150224 unless the field record
 *           the payload byte D_801FC4C0 selects in the 0x74-byte-stride table
 *           D_80143FC8 has its busy bit 0 set, then advance to 5 and 7.
 *           Sub-state 7 runs the shared front-end step func_8015C058, sets flag
 *           bit 1 of D_8014686C through func_8015B5A8, requests the selector
 *           context func_8019FA28(0xC1, 0x1A8000, 0x1C0000, 0x82) and clears
 *           the state tuple and bit 0 of the front state byte, or hands 0x98
 *           and 0x10 to func_801647C4 while the phase reads 2. Sub-states 20 and
 *           25 share one arm: while the phase is not 2 they either run
 *           func_8015B5A8 with flag 0x77 of D_80144F28 and request selector
 *           context 0x83 when the sub-state is 20, or set flags 2 and 3 of
 *           D_8014686C, request selector context 0x84 and publish 0xFF in
 *           D_80143F1E otherwise, and both finish by setting flag 0x8A of
 *           D_80144F28, publishing 0xFF in D_80143F1D and D_80143F1F, clearing
 *           the state tuple and clearing bit 0 of the front state byte. The
 *           remaining slots of the table return untouched. It takes no
 *           arguments and returns nothing; its 0x20-byte frame holds $ra at
 *           0x18 and the two outgoing argument words of the move helper at
 *           0x10 and 0x14, and no callee-saved register is used.
 *
 * Name evidence: the body is entry 1 of the reviewed primary-state handler
 * table at 0x801FC424, the byte it switches on is the shared secondary
 * sub-state byte D_80146875 that this target's other sources name as the
 * second member of the front-end state tuple, and 0x801F6C04 is the leading
 * table-data base of this payload, so the family form of
 * dispatchSharedModeByteTable2C04World04Area16813_801F4A0C names this body
 * dispatchSubStateByteTable6C04ScenarioScena1400.
 *
 * Shape: the payload holds no byte-identical copy of this 0x418-byte span -
 * a scan of the 396 extracted payloads finds none, and the span names the
 * payload-local byte D_801FC4C0 and this payload's own jump table, which no
 * other payload carries. The nearest sibling is
 * func_801FA9DC of scena15/00 (0x801FA9DC, 0x420 bytes), still on its asm row,
 * whose decoded body is instruction-for-instruction identical apart from the
 * five payload-or-bank bindings it takes from D_80144EF8 and D_801FE748 where
 * this one takes D_8014686C and D_801FC4C0 and apart from the $s0 it saves to
 * hold that bank pointer, so it supplied the statement order, operand forms and
 * declaration style while every literal here comes from this span's own bytes.
 * The jump table is placed by this target's matching section-placement row for
 * the .rodata section at 0x801F6C04.
 *
 * Representation: the two reset arms read the front state byte through a
 * pointer local and write it back after the other stores, which is what splits
 * a plain read-modify-write into the original's hoisted load and its trailing
 * store. Arm 1 loads the source byte of the D_80143F1F copy into its own local
 * before it reads the front state byte, and arm 7 uses its own pointer and
 * value locals rather than arm 1's, because one shared pair put those three
 * values in the registers the original keeps for other operands. Losing
 * spellings measured on the way here: the folded `*front |= 1` tail as the
 * arm's last statement scored 226/262 instructions with a 1056-byte body, and
 * the read-write split with one shared pointer and value pair across both arms
 * scored 248/262; the present shape scored 261/262, the remaining instruction
 * being the jump-table load that the placement row covers. Those three probes
 * ran while the section-placement row was withheld so the body was measurable;
 * it was restored before the gate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubStateByteTable6C04ScenarioScena1400_801F7BA0(void) {
  u8* front;
  u8 flag;
  u8 copied;
  u8* state;
  u8 bits;

  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2) {
      return;
    }
    g_ScenarioProgress = 0;
    func_801BE1B0(1);
    D_80146875 = 1;
    return;
  case 1:
    if (g_ScenarioProgress != 1) {
      return;
    }
    func_8015B580(D_8014686C, 1);
    func_8019FA28(0xBD, 0x10000000, 0x14000000, 2);
    front = &D_80146871;
    copied = D_80145029;
    flag = *front;
    D_8014504B = 2;
    D_80144F54 = 0;
    D_80146875 = 2;
    D_80143F1F = copied;
    *front = flag | 1;
    return;
  case 2:
    if (D_80143C40 != 0) {
      return;
    }
    func_80150224(0x12);
    D_80143BB0 = 2;
    D_80146875 = 3;
    return;
  case 3:
    if (D_80143BB0 != 2) {
      func_801F7A18(0x13, D_801492D8, D_801492DA, -0x200, 0x20, 0);
      D_80146875 = 4;
      return;
    }
    func_801647C4(0x98, 0x28, 0);
    return;
  case 4:
    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1) != 0) {
      return;
    }
    func_80150224(0x13);
    D_80143BB0 = 2;
    D_80146875 = 5;
    return;
  case 5:
    if (D_80143BB0 != 2) {
      func_801F7A18(0x13, D_801492D8, D_801492DA, 0x400, 0x20, 0);
      D_80146875 = 6;
      return;
    }
    func_801647C4(0x98, 0x38, 0);
    return;
  case 6:
    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1) != 0) {
      return;
    }
    func_80150224(0x14);
    D_80143BB0 = 2;
    D_80146875 = 7;
    return;
  case 7:
    if (D_80143BB0 != 2) {
      func_8015C058();
      func_8015B5A8(D_8014686C, 1);
      func_8019FA28(0xC1, 0x1A8000, 0x1C0000, 0x82);
      state = &D_80146871;
      bits = *state;
      D_80146874 = 0;
      D_80146875 = 0;
      *state = bits & 0xFE;
      return;
    }
    func_801647C4(0x98, 0x10, 0);
    return;
  case 20:
  case 25:
    if (D_80143BB0 == 2) {
      return;
    }
    if (D_80146875 == 20) {
      func_8015B5A8((u32)&D_80144F28, 0x77);
      D_8014504A = 0;
      func_8019FA28(0xC1, 0x1A8000, 0x1C0000, 0x83);
    } else {
      func_8015B580(D_8014686C, 2);
      func_8015B580(D_8014686C, 3);
      func_8019FA28(0xC1, 0x1A8000, 0x1C0000, 0x84);
      D_80143F1E = 0xFF;
    }
    func_8015B580((u32)&D_80144F28, 0x8A);
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146874 = 0;
    D_80146875 = 0;
    D_80146871 &= 0xFE;
  }
}
