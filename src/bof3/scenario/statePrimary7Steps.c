#include "bof3/bof3.h"

/* Shared scenario progress byte the arm gates compare against 1, 7, 9, 0xA,
 * 0x19 and 0, that several arms increment and none here clears.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte: three arms test it for 2, one tests it for 0 and
 * two publish 6 or 2 in it.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary state byte of the front-end state tuple, cleared by the six
 * reset tails and by the 7 arm.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the same tuple and the switch operand of
 * this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared tick halfword: the four countdown arms decrement it once per frame and
 * three arms seed it with 8, 0xA, 0x78 or 0x40.
 * @source 0x80146876 @kind data
 */
extern u16 D_80146876;

/* Shared flag-bank word the six seeding arms pass to func_8015B580.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

/* Shared front-end mode byte the state transitions seed with 0x1F and clear.
 * @source 0x8014832E @kind data
 */
extern u8 D_8014832E;

/* Shared front-end flag halfword the 0x14 arm tests before it starts an action.
 * @source 0x80143C40 @kind data
 */
extern u16 D_80143C40;

/* Shared selection byte the record helpers take as their selector.
 * @source 0x80145029 @kind data
 */
extern u8 D_80145029;

/* Shared surface flag byte the 0x16 arm raises.
 * @source 0x80148660 @kind data
 */
extern u8 D_80148660;

/* Shared object-row byte the 0x0F arm seeds with 0xFF.
 * @source 0x80143F1F @kind data
 */
extern u8 D_80143F1F;

/* Shared status bytes the 0x17 arm seeds before it stages the effect rows.
 * @source 0x8014933E @kind data
 */
extern u8 D_8014933E;
/* @source 0x8014933F @kind data
 */
extern u8 D_8014933F;
/* @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared position halfwords the 0x10 arm hands to the overlay move helper.
 * @source 0x801492D8 @kind data
 */
extern s16 D_801492D8;
/* @source 0x801492DA @kind data
 */
extern s16 D_801492DA;
/* @source 0x801492DC @kind data
 */
extern s16 D_801492DC;

/* Shared live work-record count byte: the 0x17 arm stages its effect rows while
 * the scratchpad index stays below it.
 * @source 0x80146254 @kind data
 */
extern u8 D_80146254;

/* Shared 0x74-byte field-record table: the object byte func_8019601C answers
 * selects the row the 0x11 and 0x17 arms claim at +0, +5, +0x34, +0x38 and
 * +0x3C.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

/* Shared 0xA4-byte-stride effect rows: the 0x17 arm copies byte 0x0C onwards of
 * the selected row, plus its destination cells, through the bases below.
 * @source 0x80144968 @kind table
 */
extern u8 D_80144968[];
/* @source 0x8014496F @kind table
 */
extern u8 D_8014496F[];
/* @source 0x80144974 @kind table
 */
extern u8 D_80144974[];
/* @source 0x8014497C @kind table
 */
extern u8 D_8014497C[];
/* @source 0x8014497E @kind table
 */
extern u8 D_8014497E[];
/* @source 0x80144980 @kind table
 */
extern u8 D_80144980[];
/* @source 0x80144984 @kind table
 */
extern u8 D_80144984[];
/* @source 0x80144986 @kind table
 */
extern u8 D_80144986[];
/* @source 0x80144992 @kind table
 */
extern u8 D_80144992[];

/* Shared scratchpad index byte that selects the effect row and the work record.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

/* Shared byte table the scratchpad index maps through.
 * @source 0x80144F5A @kind table
 */
extern u8 D_80144F5A[];

/* Shared effect-row selector table the mapped byte indexes.
 * @source 0x80181B10 @kind table
 */
extern u8 D_80181B10[];

/* Shared 0x140-byte-stride work records the 0x17 arm stages its copied rows
 * into.
 * @source 0x80145F04 @kind table
 */
extern u8 D_80145F04[];

/* The 0xA4-byte effect row the 0x17 arm copies whole into a work record: 41
 * words, so the copy is word-aligned and word-scaled.
 */
typedef struct {
  u32 words[0x29];
} EffectRecord;

u8 func_801F7A18(u32 arg0, s16 arg1, s16 arg2, s32 arg3, u8 arg4, u32 arg5);
u32 func_8019601C(void);
s32 func_8015477C(s32 x, s32 y);
u32 func_80163EA0(void);
void func_8015C058(void);
void func_8015B580(u32 arg0, s32 arg1);
void func_801BE1B0(u32 arg0);
void func_80150224(s32 action_id);
void func_801F4400(void);
void func_801F4508(void);
void func_801F4674(void);
void func_801F4764(void);
void func_80161BBC(s32 arg0);
void func_80161D20(s32 arg0);
void func_80161D74(s32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
void func_80166180(s32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
void func_801C1DF0(s32 arg0);
void func_8019FA28(s32 arg0, u32 arg1, u32 arg2, s32 arg3);
void func_801A78F8(void);

/* @source 0x801FA9C8
 * @behavior Primary-state-7 handler of this overlay and sub-state dispatcher of
 *           its front-end entry chain; the payload keeps its address once, as
 *           zero-based word 7 at 0x801FC440 of the ten-word handler table
 *           at 0x801FC424 that
 *           callPrimaryStateHandler4 indexes with
 *           the shared primary state byte D_80146874, and no in-image jal names
 *           it. It steps the shared secondary sub-state byte D_80146875 through
 *           the 33-slot jump table this payload keeps at 0x801F6F54, reading the
 *           byte with lbu, bounding it with sltiu against 0x21 and hoisting both
 *           above the frame setup; the eight unpopulated slots and every value
 *           outside the table return untouched. Sub-states 0 to 5 release the
 *           shared front end through func_8015C058, seed one flag of the shared
 *           bank D_8014686C through func_8015B580 with 0x15, 0x16, 0x17, 0x19,
 *           0x1A or 0x1C, queue the front-end action 1, 2, 3, 6, 7 or 0 through
 *           func_801BE1B0, and clear the shared state tuple; 1 to 4 then hand
 *           control to func_801F4400, func_801F4508, func_801F4674 or
 *           func_801F4764. Sub-state 6 starts a timed beat of 8 frames at
 *           progress 7 through func_80161CD0, sub-state 7 counts it down and,
 *           at zero, raises mode 0x1F and advances the progress byte, and 0x0A
 *           advances to 0x0B once func_80162D00 reports success. Sub-states 0x0B
 *           and 0x0C run the same 0x0A-frame beat with mode 0x1F, gated on
 *           progress 0x19 and on the front-end phase byte still reading 2.
 *           Sub-state 0x0D stages position 0x2C8000/0x278000 through
 *           func_801BFE34 and queues action 0x33; 0x0F stages 0xAC/0x208000/
 *           0x268000 with code 0x89 through func_8019FA28 and seeds the
 *           object-row byte 0x80143F1F; 0x10, at progress 1, drives the overlay
 *           move helper func_801F7A18 with action 0x31 over the shared position
 *           halfwords and hands 0x9F to func_80161BBC; 0x11, at progress 9,
 *           claims the free 0x74-byte field record func_8019601C answers,
 *           seeds its state, row byte, two coordinates and the distance
 *           func_8015477C reports; 0x12 starts the 0x78-frame beat. Sub-state
 *           0x14 starts action 0x22 once the shared flag halfword D_80143C40 is
 *           clear and publishes front-end phase 2; 0x15 hands 0x0B and 1 to
 *           func_801C1DF0 and yieldCallbackSlotScheduler and publishes phase 6; 0x16 raises
 *           phase 1 with the surface flag D_80148660. Sub-state 0x17 publishes
 *           phase 2, clears the status bytes, stages 0x258000/0x278000 through
 *           func_801BFE34, walks the eight 0xA4-byte effect rows from scratchpad
 *           index 1 and copies each flagged row into the 0x140-byte work records
 *           at D_80145F04 through the selector tables D_80144F5A and
 *           D_80181B10, raises mode 0x1F, queues the 0x40-frame beat and claims
 *           a second field record. Sub-state 0x18 counts that beat down and
 *           queues action 0x34. Sub-states 0x1A and 0x20 release the shared
 *           front end and clear the tuple, 0x1A raising the progress byte and
 *           staging 0xAC/0x208000/0x268000 with code 3 first. Sub-state 0x1E
 *           hands the selection byte to func_80161D20, stages the pair 2/1 and
 *           action 0xC, starts action 0x2A and publishes phase 2; 0x1F, once
 *           func_80163EA0 reports success and the phase byte moved off 2, hands
 *           the selection byte to func_80161D74 and advances to 0x20. The body
 *           takes no arguments, returns nothing and uses a 0x20-byte frame that
 *           keeps $ra at 0x1C and $s0 at 0x18, $s0 holding the field-record
 *           pointer across the func_8015477C call while the scratchpad index
 *           pointer stays in $a0 across the two walk loops. The jump table is
 *           placed by the matching section-placement row for the .rodata
 *           section at 0x801F6F54.
 * @status partial
 * @match 99.03
 * @residual Six instructions differ in the 0x17 scratchpad walks. This body
 *           materialises the shared base in $a1 rather than $v0, writes both
 *           initializers through $a1 rather than $a0, and copies $a1 rather
 *           than $a0 into $t1. All branch destinations and the 0x1F constant
 *           placement match, and the span is 2476 bytes.
 */
void statePrimary7Steps(void) {
  u8* index;
  u8* walk;
  u32 offset;

  switch (D_80146875) {
  case 0:
    func_8015C058();
    func_8015B580(D_8014686C, 0x15);
    func_801BE1B0(1);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 1:
    func_8015C058();
    func_8015B580(D_8014686C, 0x16);
    func_801BE1B0(2);
    func_801F4400();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 2:
    func_8015C058();
    func_8015B580(D_8014686C, 0x17);
    func_801BE1B0(3);
    func_801F4508();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 3:
    func_8015C058();
    func_8015B580(D_8014686C, 0x19);
    func_801BE1B0(6);
    func_801F4674();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 4:
    func_8015C058();
    func_8015B580(D_8014686C, 0x1A);
    func_801BE1B0(7);
    func_801F4764();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 5:
    func_8015C058();
    func_8015B580(D_8014686C, 0x1C);
    func_801BE1B0(0);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 6:
    if (g_ScenarioProgress != 7) {
      return;
    }
    D_8014832E = 0;
    func_80161CD0(D_80145029, 0x64, 4);
    D_80146876 = 8;
    D_80146875 = 7;
    return;
  case 7: {
    u16* counter = &D_80146876;
    u8* progress;

    if (--*counter != 0) {
      return;
    }
    progress = &g_ScenarioProgress;
    D_8014832E = 0x1F;
    D_80146874 = 0;
    D_80146875 = 0;
    *progress = *progress + 1;
    return;
  }
  case 0x0A:
    if (func_80162D00() == 0) {
      return;
    }
    func_801BE1B0(3);
    func_80161C20(0x9F, 0x64, 8);
    D_80146875 = 0x0B;
    return;
  case 0x0B:
    if (g_ScenarioProgress != 0x19) {
      return;
    }
    if (D_80143BB0 == 2) {
      return;
    }
    D_80146876 = 0x0A;
    D_8014832E = 0;
    D_80146875 = 0x0C;
    return;
  case 0x0C: {
    u16* counter = &D_80146876;
    u8* progress;

    if (--*counter != 0) {
      return;
    }
    progress = &g_ScenarioProgress;
    D_8014832E = 0x1F;
    D_80146875 = 0x0D;
    *progress = *progress + 1;
    return;
  }
  case 0x0D:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_801BFE34(0x2C8000, 0x278000, 0x25);
    func_801C7A54(0x33);
    D_80146875 = 0x0E;
    return;
  case 0x0F:
    func_8019FA28(0xAC, 0x208000, 0x268000, 0x89);
    D_80143F1F = 0xFF;
    D_80146875 = 0x10;
    return;
  case 0x10:
    if (g_ScenarioProgress != 1) {
      return;
    }
    func_801F7A18(0x31, D_801492D8, D_801492DA, D_801492DC, 0x14, 0);
    func_80161BBC(0x9F);
    D_80146875 = 0x11;
    return;
  case 0x11: {
    u8* rec;
    u8 row;

    if (g_ScenarioProgress != 9) {
      return;
    }
    row = func_8019601C();
    if (row != 0xFF) {
      rec = D_80143FC8 + (u32)row * 0x74u;
      *(u32*)(rec + 0x34) = 0x1C8000;
      rec[0] = 1;
      rec[5] = 0x97;
      *(u32*)(rec + 0x38) = 0x268000;
      *(u32*)(rec + 0x3C) =
          (func_8015477C(*(s32*)(rec + 0x34), 0x268000) << 16) + 0x800000;
    }
    D_80146875 = 0x12;
    return;
  }
  case 0x12:
    if (g_ScenarioProgress != 0x0A) {
      return;
    }
    D_80146876 = 0x78;
    D_80146875 = 0x13;
    return;
  case 0x14: {
    u8* progress;

    if (D_80143C40 != 0) {
      return;
    }
    progress = &g_ScenarioProgress;
    D_8014832E = 0;
    *progress = *progress + 1;
    func_80150224(0x22);
    D_80143BB0 = 2;
    D_80146875 = 0x15;
    return;
  }
  case 0x15: {
    u8* phase = &D_80143BB0;

    if (*phase == 2) {
      return;
    }
    func_801C1DF0(0x0B);
    yieldCallbackSlotScheduler(1);
    *phase = 6;
    D_80146875 = 0x16;
    return;
  }
  case 0x16: {
    u8* phase = &D_80143BB0;

    if (*phase != 0) {
      return;
    }
    *phase = 1;
    D_80148660 = 1;
    D_80146875 = 0x17;
    return;
  }
  case 0x17: {
    u8* rec;
    u8 row;
    struct ScratchIndex { u8 index; };
    struct ScratchIndex* scratch;

    if (D_80143BB0 != 0) {
      return;
    }
    D_8014933E = 4;
    D_8014933F = 0;
    D_80149333 = 2;
    func_801BE1B0(0x0A);
    func_801BFE34(0x258000, 0x278000, 0x22);
    scratch = (struct ScratchIndex*)D_1F800000;
    index = &scratch->index;
    scratch->index = 1;
    do {
      offset = (u32)*index * 0xA4u;
      if ((D_8014496F[offset] & 1) != 0) {
        D_80144980[offset] = D_80144992[offset];
        offset = (u32)*index * 0xA4u;
        *(u16*)(D_8014497C + offset) = *(u16*)(D_80144984 + offset);
        offset = (u32)*index * 0xA4u;
        *(u16*)(D_8014497E + offset) = *(u16*)(D_80144986 + offset);
        offset = (u32)*index * 0xA4u;
        *(u16*)(D_80144974 + offset) = 0;
      }
      *index = *index + 1;
    } while (*index < 8);
    scratch->index = 1;
    walk = &scratch->index;
    while (*walk < D_80146254) {
      EffectRecord* row =
          (EffectRecord*)(D_80144968 +
                          (u32)D_80181B10[D_80144F5A[*walk]] * 0xA4u);

      *(EffectRecord*)(D_80145F04 + (u32)*walk * 0x140u) = *row;
      *walk = *walk + 1;
    }
    D_8014832E = 0x1F;
    func_8014ECAC(1);
    D_80146876 = 0x40;
    D_80146875 = 0x18;
    row = func_8019601C();
    if (row != 0xFF) {
      rec = D_80143FC8 + (u32)row * 0x74u;
      *(u32*)(rec + 0x34) = 0x1C8000;
      rec[0] = 1;
      rec[5] = 0x98;
      *(u32*)(rec + 0x38) = 0x268000;
      *(u32*)(rec + 0x3C) =
          (func_8015477C(*(s32*)(rec + 0x34), 0x268000) << 16) + 0x800000;
    }
    return;
  }
  case 0x18: {
    u16* counter = &D_80146876;

    if (--*counter != 0) {
      return;
    }
    func_801C7A54(0x34);
    D_80146875 = 0x19;
    return;
  }
  case 0x1A: {
    u8* progress = &g_ScenarioProgress;

    *progress = *progress + 1;
    func_8015B580(D_8014686C, 0x1F);
    func_8019FA28(0xAC, 0x208000, 0x268000, 3);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  }
  case 0x1E:
    func_80161D20(D_80145029);
    func_801636A0(2, 1);
    func_80166180(0x0C);
    func_80150224(0x2A);
    D_80143BB0 = 2;
    D_80146875 = 0x1F;
    return;
  case 0x1F: {
    u8* progress;

    if ((u8)func_80163EA0() == 0) {
      return;
    }
    if (D_80143BB0 == 2) {
      return;
    }
    func_80161D74(D_80145029);
    progress = &g_ScenarioProgress;
    D_80146875 = 0x20;
    *progress = *progress + 1;
    return;
  }
  case 0x20:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_801A78F8();
    func_8015B580(D_8014686C, 0x31);
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  }
}
