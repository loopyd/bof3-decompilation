#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: the waiting arms of
 * this walker compare it against one value each and two of them clear it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared mode halfword of the main executable: sub-state 0x10 sets bit 0x08 and
 * sub-state 0x20 toggles the same bit, both with lhu/ori| xori/sh.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared mode byte of the main executable: sub-state 0 waits for it to stop
 * reading 3 and no arm stores it.
 * @source 0x8014623B @kind unknown */
extern u8 D_8014623B;

/* Shared armed-state byte of the main executable: sub-states 0x0E and 0x2E arm
 * it with 2 and sub-states 0x0F and 0x2F wait for it to read 0.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared effect-bank byte of the main executable: sub-state 0x2D clears it and
 * no arm reads it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared gate halfword of the main executable: sub-state 0x2D waits for it to
 * read 0.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared main-RAM byte of the main executable: sub-state 0x12 tests it against
 * the sentinel 0xFF and sub-state 2 clears it with the other window bytes.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared main-RAM byte of the main executable: sub-state 0x12 clears it unless
 * it reads 0x80, sub-state 0x2F clears it, and sub-state 2 clears it too.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared main-RAM byte of the main executable: sub-state 0x0C publishes the
 * record index in it, sub-state 0x0D reads it back and sub-state 2 clears it.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario flag word of the main executable: every arm that sets a
 * scenario bit passes it to func_8015B580.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared state byte of the main executable: the overlay handler run at
 * 0x801FD1C4 is indexed with it and the closing arms clear it to return the
 * overlay to its idle entry.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte of the main executable: every arm of this walker reads
 * it once through the jump table and most of them store the next value into it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared gate byte of the main executable: sub-states 0x0D, 0x11, 0x14, 0x1E
 * and 0x2D hold on while it reads non-zero.
 * @source 0x80149332 @kind unknown */
extern u8 D_80149332;

/* Shared halfword of the main executable copied into the record word at offset
 * 0x64 by sub-state 0x2C.
 * @source 0x801492D8 @kind unknown */
extern s16 D_801492D8;

/* Shared halfword of the main executable copied into the record word at offset
 * 0x68 by sub-states 0x0B, 0x0C, 0x22 and 0x2C.
 * @source 0x801492DA @kind unknown */
extern s16 D_801492DA;

/* Shared list of one byte per active slot of the main executable: sub-state
 * 0x0E walks its first three entries four times and compares each against one
 * wanted id.
 * @source 0x80144F5A @kind data */
extern u8 D_80144F5A[];

/* Shared halfword of the main executable tested by sub-state 0x12 against the
 * three window values 0x2000, 0x3000 and 0x6000.
 * @source 0x80145AA4 @kind unknown */
extern u16 D_80145AA4;

/* Shared word of the main executable added to 0x30000 for the first context
 * word sub-state 0x1F requests.
 * @source 0x80146144 @kind data */
extern u32 D_80146144;

/* Scratchpad byte at 0x1F800000: the record-filling arms store the slot query
 * result here and then index the record run with this byte. Every read after a
 * record store is a real lbu, because that store may alias the byte.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* One 0x74-byte field record of the shared main-RAM run the arms fill in from
 * the slot query.
 * @kind struct */
typedef struct {
  u8 unk_00;        /* 0x00 slot active byte, stored as 1 */
  u8 unk_01[4];     /* 0x01 */
  u8 unk_05;        /* 0x05 arm byte, stored as 0x13, 0x82, 0x83 or 0x84 */
  u8 unk_06[3];     /* 0x06 */
  u8 unk_09;        /* 0x09 sub-state byte, stored as 0x28 or 0x14 */
  u8 unk_0A[0x5A];  /* 0x0A */
  s32 unk_64;       /* 0x64 stored as -0x32E, -0x2AA, -0x35A or D_801492D8 */
  s32 unk_68;       /* 0x68 copied from D_801492DA */
  s32 unk_6C;       /* 0x6C stored as 0x8A, 0x200 or 0x2B0 */
  u8 unk_70[4];     /* 0x70 */
} FieldRecord;

/* Base of the shared 0x74-byte field-record run of the main executable, indexed
 * by the slot byte the arms publish at 0x1F800000.
 * @source 0x80143FC8 @kind data */
extern FieldRecord D_80143FC8[];

/* Shared slot query at 0x8019601C of the main executable: it answers the next
 * free 0..0xFE record index, with 0xFF meaning none, and the filling arms
 * publish the answer in the scratchpad byte before they index with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared mode-and-effect selector at 0x801BE1B0: sub-states 0, 0x0C, 0x12, 0x14
 * and 0x1E run it with one selector each.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared scenario flag helper at 0x8015B580: the arms that set a scenario bit
 * pass it the shared flag word and the bit number.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared front-end release helper at 0x8015C058: sub-states 1, 2 and 0x20 run
 * it before they clear the window bytes or the whole state.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared two-argument cell setter at 0x8015D404: sub-state 0x0A runs it once,
 * with (0, 0).
 * @source 0x8015D404 @kind function */
extern void func_8015D404(u32 arg0, s32 arg1);

/* Shared front-end cue builder at 0x80161C20: sub-state 0x0A runs it with the
 * selection 0xA4, the level 0x64 and the shape 8.
 * @source 0x80161C20 @kind function */
extern void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);

/* Shared overlay reset helper at 0x801C601C of the main executable: each
 * sub-state that steps from one window to the next runs it with the selector
 * the original encodes as 0, 2, 3 or 4.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared action trigger at 0x80150224: sub-state 0x0E runs it with the ids 5,
 * 4, 3 and 2 and sub-state 0x2E with the id 0xB.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared two-word front-end request at 0x801BFE34: sub-states 0x15 and 0x1F
 * pass it a pair of context words and the selection 0x26.
 * @source 0x801BFE34 @kind function */
extern void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);

/* Shared context queue at 0x801C7A54: sub-states 0x16 and 0x1F run it with the
 * selection 0x26.
 * @source 0x801C7A54 @kind function */
extern void func_801C7A54(s32 arg0);

/* Shared front-end mode setter at 0x8014ECAC: sub-state 0x2B sets mode 0xD
 * through it.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared world teardown at 0x801A78F8: sub-state 0x2F runs it with no
 * arguments before it requests the last front-end context.
 * @source 0x801A78F8 @kind function */
extern void func_801A78F8(void);

/* Shared front-end request setter at 0x8019FA28: sub-state 0x2F passes it the
 * 16-bit selection seed 0x7E, the two 16.16 context words and the request kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* @source 0x801FB864
 * @behavior Overlay secondary-state handler: it is the word stored at
 * 0x801FD1E8, entry 9 of the ten-word handler run at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 9;
 * that table word is the only image reference to this address, no jal or j in
 * the payload targets it, and the closing arms clear D_80146874 themselves so
 * the overlay returns to its idle entry. It switches on the shared sub-state
 * byte D_80146875 through the forty-eight-entry absolute jump table at
 * 0x801F70B4, an overlay-local table of the leading tabledata blob, so
 * sub-states 0 through 0x2F are its table entries and every hole and every
 * value above 0x2F returns untouched. Sub-state 0 waits for the shared mode
 * byte D_8014623B to stop reading 3 and runs func_801BE1B0 (0x801BE1B0) with 0.
 * Sub-state 1 waits for the shared progress byte g_ScenarioProgress (0x80146864)
 * to read 1, sets scenario bit 0x30 through func_8015B580 (0x8015B580),
 * releases the front end through func_8015C058 (0x8015C058) and clears the
 * progress, the sub-state and the state byte; sub-state 2 releases the front end
 * and clears the progress, the three window bytes, the sub-state and the state
 * byte; and sub-state 0x20 waits for progress 5, sets bit 0x2D through
 * func_8015B580, toggles bit 0x08 of the shared mode halfword D_80146258,
 * releases the front end and runs the same clearing tail. Sub-states 0x0B, 0x0C,
 * 0x22 and 0x2C wait for the progress values 2, 6, 1 and 3, take a slot index
 * from the shared query func_8019601C (0x8019601C), publish it in the scratchpad
 * byte 0x1F800000 and fill that entry of the shared 0x74-byte record run at
 * 0x80143FC8 with the active byte 1, the arm byte 0x13 and their own image,
 * height and sub-state words; sub-state 0x0C also publishes the index in
 * D_80146867, which sub-state 0x0D reads before it waits for the shared gate
 * byte D_80149332 to read 0 and stores 7 in the progress byte. Sub-state 0x0E
 * walks the first three entries of the shared list byte run D_80144F5A four
 * times and runs the shared action trigger func_80150224 (0x80150224) with the
 * ids 5, 4, 3 and 2 for the entries that read 2, 8, 5 and 4, then arms
 * D_80143BB0 with 2; sub-state 0x0F waits for that byte to read 0 and steps the
 * progress byte to 0x0A. Sub-state 0x10 steps to 0x11 and sets mode bit 0x08;
 * sub-state 0x11 waits for progress 0x0E and, while D_80149332 stays clear,
 * queries three slots and fills them with the arm bytes 0x82, 0x83 and 0x84
 * before it steps to 0x12. Sub-state 0x12 waits for the window byte D_80146865
 * to leave 0xFF and switches on the shared halfword D_80145AA4: values 0x2000,
 * 0x3000 and 0x6000 run func_801BE1B0 with 7 and the overlay reset helper
 * func_801C601C (0x801C601C) with selector 3, and every other value clears
 * D_80146866 unless it already reads 0x80, then steps to 0x12 or 0x13.
 * Sub-states 0x14, 0x1E, 0x2B and 0x2D gate on D_80149332, progress 2, the
 * shared gate halfword D_80143C40 and the byte D_80143BB0, and run
 * func_801BE1B0 with 4 and 3, func_801C601C with selectors 4 and 2, set
 * front-end mode 0xD through func_8014ECAC (0x8014ECAC), set scenario bit 0x2D
 * through func_8015B580 and clear the shared effect-bank byte D_8014832E.
 * Sub-states 0x15 and 0x16 wait for progress 0x18 and 0x19 and request a
 * context pair through func_801BFE34 (0x801BFE34) with the words 0x348000 and
 * 0x258000, or D_80146144 plus 0x30000 and 0x258000, and the selection 0x26,
 * which sub-state 0x16 and 0x1F also queue through func_801C7A54 (0x801C7A54).
 * Sub-state 0x2E runs func_80150224 with 0xB and arms D_80143BB0 with 2, and
 * sub-state 0x2F waits for that byte to read 0, clears the window byte, the
 * sub-state and the state byte, sets scenario bit 0x3F through func_8015B580,
 * runs func_801A78F8 (0x801A78F8) and requests the front-end context 0x7E
 * through func_8019FA28 (0x8019FA28) with the words 0x500000 and 0x1E0000 and
 * the request kind 4. Its 0xD10 bytes have no copy in the tree: a full-span
 * probe over every payload under out/binaries matched no second file, so the
 * body was written from this span's own bytes with the statement order, operand
 * forms and declaration style of the same-target sibling
 * stepSubstatesThroughFrontEndContextsAndFieldRecordSlotsThenReleaseStateScenarioScena1200_801F9F0C
 * (0x801F9F0C), which measured exact when this seed was written and carries the
 * same switch on D_80146875, the same progress-wait arms and the same 0x74-byte
 * record fill driven by the scratchpad slot byte. The original shares one
 * clearing tail between sub-states 1, 2 and 0x20 and one sub-state store between
 * the two arms of sub-state 0x12; the source spells those tails once per arm and
 * the compiler cross-jumps them back together. The frame keeps $ra, $s1 and
 * $s0, the latter for the scratchpad byte pointer the three fills of sub-state
 * 0x11 and the list loops of sub-state 0x0E keep across their calls, and $s1 for
 * the 0xFF sentinel and the list compare values.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSubstatesSeedingFieldRecordsRequestingFrontEndContextsThenReleaseFrontEndAndResetStateScenarioScena1200_801FB864(void) {
  switch (D_80146875) {
  case 0:
    if (D_8014623B == 3) {
      break;
    }
    func_801BE1B0(0);
    D_80146875 = 1;
    break;
  case 1: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    func_8015B580(D_8014686C, 0x30);
    func_8015C058();
    *progress = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  }
  case 2:
    func_8015C058();
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 10:
    func_8015D404(0, 0);
    func_80161C20(0xA4, 0x64, 8);
    func_801BE1B0(1);
    func_801C601C(0);
    D_80146875 = 0xB;
    break;
  case 11:
    if (g_ScenarioProgress != 2) {
      break;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) == 0xFF) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x32E;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x8A;
    D_80143FC8[D_1F800000[0]].unk_09 = 0x28;
    D_80146875 = 0xC;
    break;
  case 12:
    if (g_ScenarioProgress != 6) {
      break;
    }
    func_801BE1B0(2);
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) == 0xFF) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x2AA;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x200;
    D_80143FC8[D_1F800000[0]].unk_09 = 0x28;
    D_80146875 = 0xD;
    D_80146867 = D_1F800000[0];
    break;
  case 13:
    if (D_80143FC8[D_80146867].unk_00 != 0) {
      break;
    }
    if (D_80149332 != 0) {
      break;
    }
    g_ScenarioProgress = 7;
    D_80146875 = 0xE;
    break;
  case 14: {
    u8 i;
    if (g_ScenarioProgress != 9) {
      break;
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 2) {
        func_80150224(5);
      }
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 8) {
        func_80150224(4);
      }
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 5) {
        func_80150224(3);
      }
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 4) {
        func_80150224(2);
      }
    }
    D_80143BB0 = 2;
    D_80146875 = 0xF;
    break;
  }
  case 15:
    if (D_80143BB0 != 0) {
      break;
    }
    g_ScenarioProgress = 0xA;
    D_80146875 = 0x10;
    break;
  case 16:
    D_80146875 = 0x11;
    D_80146258 |= 8;
    break;
  case 17:
    if (g_ScenarioProgress != 0xE) {
      break;
    }
    if (D_80149332 != 0) {
      break;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) != 0xFF) {
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x82;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) != 0xFF) {
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x83;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) != 0xFF) {
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x84;
    }
    D_80146875 = 0x12;
    break;
  case 18:
    if (D_80146865 == 0xFF) {
      break;
    }
    if (D_80145AA4 == 0x3000 || D_80145AA4 == 0x6000 ||
        D_80145AA4 == 0x2000) {
      if (D_80149332 != 0) {
        break;
      }
      func_801BE1B0(7);
      func_801C601C(3);
      D_80146875 = 0x12;
      break;
    }
    if (D_80146866 != 0x80) {
      D_80146866 = 0;
      D_80146875 = 0x12;
      break;
    }
    D_80146875 = 0x13;
    break;
  case 20:
    if (D_80149332 != 0) {
      break;
    }
    func_801BE1B0(4);
    func_801C601C(4);
    D_80146875 = 0x15;
    break;
  case 21:
    if (g_ScenarioProgress != 0x18) {
      break;
    }
    D_80146875 = 0x16;
    func_8015B580(D_8014686C, 2);
    func_801BFE34(0x348000, 0x258000, 0x26);
    break;
  case 22:
    if (g_ScenarioProgress != 0x19) {
      break;
    }
    D_80146875 = 0x17;
    func_801C7A54(0x26);
    break;
  case 30:
    if (D_80149332 != 0) {
      break;
    }
    func_801BE1B0(3);
    D_80146875 = 0x1F;
    break;
  case 31:
    if (g_ScenarioProgress != 1) {
      break;
    }
    func_801BFE34(D_80146144 + 0x30000, 0x258000, 0x26);
    func_801C7A54(0x26);
    D_80146875 = 0x20;
    break;
  case 32:
    if (g_ScenarioProgress != 5) {
      break;
    }
    func_8015B580(D_8014686C, 0x2D);
    D_80146258 ^= 8;
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 33:
    g_ScenarioProgress = 0;
    func_801C601C(2);
    func_8015B580(D_8014686C, 0x2D);
    D_80146875 = 0x22;
    break;
  case 34:
    if (g_ScenarioProgress != 1) {
      break;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) == 0xFF) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x35A;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x200;
    D_80143FC8[D_1F800000[0]].unk_09 = 0x14;
    D_80146875 = 0x2B;
    break;
  case 43:
    if (g_ScenarioProgress != 2) {
      break;
    }
    func_8014ECAC(0xD);
    D_80146875 = 0x2C;
    break;
  case 44:
    if (g_ScenarioProgress != 3) {
      break;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) == 0xFF) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = D_801492D8;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x2B0;
    D_80143FC8[D_1F800000[0]].unk_09 = 0x14;
    D_80146875 = 0x2D;
    break;
  case 45:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    D_80146875 = 0x2E;
    break;
  case 46:
    func_80150224(0xB);
    D_80143BB0 = 2;
    D_80146875 = 0x2F;
    break;
  case 47:
    if (D_80143BB0 != 0) {
      break;
    }
    D_80146866 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    func_8015B580(D_8014686C, 0x3F);
    func_801A78F8();
    func_8019FA28(0x7E, 0x500000, 0x1E0000, 4);
    break;
  }
}
