#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: the waiting arms of
 * this walker each compare it against one value, and every arm that leaves the
 * walker clears it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared main-RAM work byte of the main executable: four arms compare it
 * against 2 and refuse to act while it reads that, two arms store 2 after
 * queueing their action, and sub-state 2 takes the cell's address once and
 * re-reads and stores it through that pointer across two calls.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared mode halfword of the main executable: sub-state 5 sets bit 0x20 of it
 * and the gate arm at sub-state 0x2C toggles bit 0x0E, both with lhu/ori or
 * lhu/xori/sh.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared gate halfword of the main executable: the arm at sub-state 0x2C
 * returns while it is nonzero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared selector halfword of the main executable: the arm at sub-state 0x19
 * compares it against the three ids 0x85, 0x87 and 0x88 and runs one
 * effect-bank step per id.
 * @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;

/* Shared main-RAM work byte of the main executable: the request arms mask it
 * with 0x7F and select one front-end request per masked value, and the gate arm
 * at sub-state 0x2C masks it the same way to select one sound-cue pair.
 * @source 0x80145024 @kind unknown */
extern u8 D_80145024;

/* Shared main-RAM byte of the main executable: the gate arm at sub-state 0x2C
 * clears it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared main-RAM byte of the main executable: the request arms store the small
 * selector 0, 1 or 2 into it before they ask for a front-end context, and every
 * arm that clears the shared state stores 0 in it first.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared main-RAM byte of the main executable: the request arms store the
 * next-window byte into it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared main-RAM byte of the main executable: every arm that clears the shared
 * state stores 0 in it before it runs the overlay release helper; this walker
 * never publishes a record index there.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared flag word of the main executable: the arms that set or clear one
 * scenario flag pass this word and the flag id to the flag helper.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared second-level state byte of the main executable: the last arm,
 * sub-state 0x2D, stores 6 in it, which hands the window to entry 6 of the same
 * handler run, and every other arm that leaves the walker clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte of the main executable: every arm of this walker reads
 * it once through the jump table and most of them store the next value into it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Main-RAM flag bank base of the main executable: the arm at sub-state 0x16
 * clears flag 0x29 of that bank, which the other arms reach through the flag
 * word above.
 * @source 0x80144F28 @kind data */
extern u8 D_80144F28[];

/* Base of the shared 0x74-byte field-record run of the main executable. The
 * two record arms scale the index the slot query answers by 0x74, form that
 * base once and store through record bytes 0, 5 and 1 in that order: bytes 0 and
 * 5 take 1 and 0x77 in both arms, and byte 1 takes 2 in the sub-state 2 arm and
 * 0 in the arm sub-states 9 and 0x16 share.
 * @source 0x80143FC8 @kind data */
extern u8 D_80143FC8[];

/* Low byte of the overlay-local halfword at 0x801FD288: the arm at sub-state
 * 0x14 reads it with lbu and tests it against the window 0x62 through 0x66.
 * @source 0x801FD288 @kind data */
extern u8 D_801FD288;

/* Shared word-at-a-time slot query at 0x8019601C of the main executable: it
 * answers the next free 0..0xFE record index, with 0xFF meaning none.
 * @source 0x8019601C @kind function */
extern u8 func_8019601C(void);

/* Shared front-end action queue at 0x80150224: the first arm queues action 2,
 * the record arm queues action 0xD and the arm at sub-state 0x1B queues 0x13.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared scenario flag helper at 0x8015B580: the arms that leave the walker
 * pass it the flag word or the bank base above and the flag id to set.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 flag_bank, u32 bit_index);

/* Shared scenario flag helper at 0x8015B5A8: the arm at sub-state 0x16 clears
 * flag 0x14 of the shared flag word through it.
 * @source 0x8015B5A8 @kind function */
extern void func_8015B5A8(u32 flag_bank, u32 bit_index);

/* Shared front-end request setter at 0x8019FA28: every request arm passes it a
 * 16-bit selection seed, the two 16.16 context words and the request kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared effect-bank stepper at 0x801BE1B0: the arms at sub-states 5, 0x14 and
 * 0x19 pass one effect id each.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared window reset helper at 0x801C601C: the arms at sub-states 7 and 0x2A
 * pass the window selector 0 and 1.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared front-end cue helper at 0x8014ECAC: the arm at sub-state 0x2B passes
 * the constant 4.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared sound-cue dispatch at 0x801C1E3C: the gate arm at sub-state 0x2C runs
 * it once or twice per masked work byte before the second helper.
 * @source 0x801C1E3C @kind function */
extern void func_801C1E3C(u8 index);

/* Shared sound-cue dispatch at 0x801C1DF0: the gate arm at sub-state 0x2C runs
 * it once per masked work byte with the id 6 or 7.
 * @source 0x801C1DF0 @kind function */
extern void func_801C1DF0(u8 index);

/* Shared overlay release helper at 0x8015C058: every arm that leaves the walker
 * by releasing the front end runs it before clearing the state bytes.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* @source 0x801F9680
 * @behavior Overlay second-level state handler: it is the word at 0x801FD1D8,
 * entry 5 of the ten-word handler run at 0x801FD1C4 that
 * callSecondLevelState (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 5.
 * That table word is the only image reference to this address: no jal or j of
 * the payload targets it. It switches on the shared sub-state byte D_80146875
 * through the forty-six-word absolute jump table jtbl_801F6DC4_main at
 * 0x801F6DC4, an overlay-local table of this payload leading tabledata blob,
 * bounded by sltiu against 0x2E, so sub-states 0 through 0x2D are its entries
 * and table entries 4, 10 through 0x13, 0x17, 0x18 and 0x1D through 0x27 point
 * at the body exit, where those values return without touching any byte.
 *
 * Sub-state 0 queues front-end action 2, stores 2 in the work byte D_80143BB0
 * and advances to sub-state 1. Sub-state 1 waits for that work byte to leave 2
 * and then clears the shared progress byte g_ScenarioProgress, the bytes
 * 0x80146865 and 0x80146866, the byte 0x80146867, runs the overlay release
 * helper func_8015C058 (0x8015C058) and clears the sub-state byte and the state
 * byte D_80146874. Sub-state 2 takes the work byte address once: while that
 * byte is not 2 it asks the shared slot query func_8019601C (0x8019601C) for a
 * record index, and when the answer is not 0xFF it forms the 0x74-byte record
 * base of the run D_80143FC8 (0x80143FC8) from that index and stores 1 in
 * record byte 0, 0x77 in record byte 5 and 2 in record byte 1, then queues
 * action 0xD and stores 2 through the saved work byte address; both paths then
 * advance to sub-state 3. Sub-state 3 waits for the work byte to leave 2, sets
 * flag 0x14 through func_8015B580 (0x8015B580), stores 10 in the progress byte,
 * stores 0 in the byte 0x80146865 and 10 in the byte 0x80146866 and runs the same
 * release-and-clear tail as sub-state 1.
 *
 * Sub-state 5 sets bit 0x20 of the shared mode halfword D_80146258, runs the
 * effect-bank stepper func_801BE1B0 (0x801BE1B0) with 3 and advances to
 * sub-state 6. Sub-state 6 waits for the progress byte to read 4, advances to
 * sub-state 7, stores 2 in the byte 0x80146866 and requests the front-end
 * context 0x88, 0x40000, 0x10000 with kind 0x84 through func_8019FA28
 * (0x8019FA28). Sub-state 7 runs func_801C601C (0x801C601C) with 0 and
 * advances to sub-state 8. Sub-state 8 waits for the progress byte to read 10,
 * sets flag 0x24, clears the progress byte, stores 1 in the byte 0x80146866,
 * advances to sub-state 9 and requests the context 0x88, 0xC0000, 0x590000
 * with kind 0x85. Sub-state 9 waits for the progress byte to read 10, sets flag
 * 0x25, clears the bytes 0x80146865 and 0x80146867, stores 10 in the byte
 * 0x80146866, runs the release helper, clears the sub-state and state bytes and
 * then seeds one field record exactly as sub-state 0x16 does.
 *
 * Sub-state 0x14 waits for the work byte to leave 2, stores 1 in the progress
 * byte and tests the low byte of the overlay-local halfword D_801FD288 against
 * the window 0x62 through 0x66: inside it runs func_801BE1B0 with 9, sets flag
 * 0x16 and advances to sub-state 0x28, while outside it runs func_801BE1B0
 * with 6 and advances to sub-state 0x15. Sub-state 0x15 waits for the progress
 * byte to read 0x23, clears it, stores 1 in the byte 0x80146865, advances to
 * sub-state 0x16 and requests the context 0x88, 0xC0000, 0x590000 with kind
 * 0x87. Sub-state 0x16 waits for the progress byte to read 10 and leaves that
 * byte standing, clears the bytes 0x80146865 and 0x80146867, stores 10 in the
 * byte 0x80146866, runs the release helper, clears the sub-state and state bytes,
 * sets flag 0x29 of the bank base D_80144F28 (0x80144F28), clears flag 0x14 of
 * the shared flag word and then answers the slot query and seeds the record
 * base with 1, 0x77 and 0 in record bytes 0, 5 and 1, the third store being the
 * zero the exit delay slot carries.
 *
 * Sub-state 0x19 advances to sub-state 0x1A and selects one effect-bank step
 * from the shared halfword D_80143F00: 0x85 runs func_801BE1B0 with 4, 0x87
 * with 2 and 0x88 with 8. Sub-state 0x1A waits for the progress byte to read
 * 0x28, clears the progress byte, stores 1 in the byte 0x80146865 and 0 in the
 * byte 0x80146866, advances to sub-state 0x16 and requests the same 0x87
 * context as sub-state 0x15, in the statement order that sub-state 0x16 shares.
 *
 * Sub-state 0x1B queues front-end action 0x13, stores 2 in the work byte and
 * advances to sub-state 0x1C, which waits for the work byte to leave 2, runs
 * the release helper and clears the state byte before the sub-state byte.
 * Sub-state 0x28 waits for the progress byte to read 0x14, clears it, stores 3
 * in the byte 0x80146866, advances to sub-state 0x29, sets flag 0x26 and then
 * selects the front-end request kind pair from the masked work byte
 * D_80145024 with 0x7F: id 0xD stores 1 in the byte 0x80146865, id 8 stores 0,
 * id 0x10 stores 2, all three requesting the context 0x85, 0x60000, 0x5C0000
 * with kind 0x86, and id 0x11 stores 0 and requests that same context with kind
 * 0x85. Sub-state 0x29 waits for the progress byte to read 10, advances to
 * sub-state 0x2A, clears the progress byte and the byte 0x80146866, sets flag
 * 0x27 and then runs the context 0x85, 0x318000, 0x268000 with kind 0x87 for
 * masked id 8 or with kind 0x88 for masked id 0x11. Sub-state 0x2A clears the
 * byte 0x80146865, runs func_801C601C with 1 and advances to sub-state 0x2B,
 * which waits for the progress byte to read 5, runs func_8014ECAC
 * (0x8014ECAC) with 4 and advances to sub-state 0x2C. Sub-state 0x2C returns
 * while the gate halfword D_80143C40 is nonzero; otherwise it advances to
 * sub-state 0x2D, toggles bit 0x0E of the mode halfword, clears the progress
 * byte and the byte 0x8014832E, and selects one sound-cue pair from the masked
 * work byte through the twelve-word absolute jump table jtbl_801F6E7C_main at
 * 0x801F6E7C, bounded by sltiu against 0xC over the ids 7 through 0x12: ids 7,
 * 0xA and 0xF run func_801C1E3C (0x801C1E3C) twice, ids 8, 9, 0xD, 0xE, 0x11
 * and 0x12 once, and every selected id then runs func_801C1DF0 (0x801C1DF0)
 * with 6, except ids 8, 0xD and 0x11, which run it with 7. Sub-state 0x2D
 * clears the progress byte, the bytes 0x80146865, 0x80146866, 0x80146867 and
 * the sub-state byte and stores 6 in the state byte, which hands the window to
 * entry 6 of the same handler run,
 * stepSubstatesThroughContexts.
 *
 * It takes no arguments and returns nothing; its 0x18-byte frame keeps $ra and
 * $s0, and $s0 holds the work byte address sub-state 2 takes once and reuses
 * across the two calls. Every cell, table and helper above is addressed from
 * the main executable except the overlay-local halfword D_801FD288 and the two
 * jump tables, so the span has no reusable in-payload body: a byte search of
 * the 398 extracted payloads under out/binaries for the whole 0x88C-byte span
 * finds this payload only, so the clone lever was declined and every literal
 * here was written from this span's own bytes. The shape came from exact
 * neighbours of the same target: stepSubstatesThroughFrontEndContextsAndField
 * RecordSlotsThenReleaseStateScenarioScena1200_801F9F0C (0x801F9F0C) supplies
 * the per-cell declaration form, the pointer local that keeps a guarded cell
 * address in a saved register across a call, and the record-seeding statement
 * order, and statePendingWorldSetup
 * (0x801F7F78) supplies the masked-work-byte switch over the ids 7 through 0x12
 * whose arms call func_801C1E3C and func_801C1DF0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void
stepSeedingRequestingOff(
    void) {
  switch (D_80146875) {
  case 0:
    func_80150224(2);
    D_80143BB0 = 2;
    D_80146875 = 1;
    break;
  case 1:
    if (D_80143BB0 == 2u) {
      break;
    }
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 2: {
    u8 *workByte = &D_80143BB0;
    u8 index;

    if (*workByte != 2u) {
      index = func_8019601C();
      if (index != 0xFF) {
        u8 *record = D_80143FC8 + (u32)index * 0x74u;

        record[0] = 1;
        record[5] = 0x77;
        record[1] = 2;
      }
      func_80150224(0xD);
      *workByte = 2;
    }
    D_80146875 = 3;
    break;
  }
  case 3:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8015B580(D_8014686C, 0x14);
    g_ScenarioProgress = 10;
    D_80146865 = 0;
    D_80146866 = 10;
    D_80146867 = 0;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 5:
    D_80146258 |= 0x20;
    func_801BE1B0(3);
    D_80146875 = 6;
    break;
  case 6:
    if (g_ScenarioProgress != 4u) {
      break;
    }
    D_80146875 = 7;
    D_80146866 = 2;
    func_8019FA28(0x88, 0x40000, 0x10000, 0x84);
    break;
  case 7:
    func_801C601C(0);
    D_80146875 = 8;
    break;
  case 8: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 10u) {
      break;
    }
    *progress = 0;
    D_80146866 = 1;
    D_80146875 = 9;
    func_8015B580(D_8014686C, 0x24);
    func_8019FA28(0x88, 0xC0000, 0x590000, 0x85);
    break;
  }
  case 9: {
    u8 index;

    if (g_ScenarioProgress != 10u) {
      break;
    }
    func_8015B580(D_8014686C, 0x25);
    D_80146865 = 0;
    D_80146866 = 10;
    D_80146867 = 0;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    index = func_8019601C();
    if (index != 0xFF) {
      u8 *record = D_80143FC8 + (u32)index * 0x74u;

      record[0] = 1;
      record[5] = 0x77;
      record[1] = 0;
    }
    break;
  }
  case 0x14: {
    u8 counter;

    if (D_80143BB0 == 2u) {
      break;
    }
    counter = D_801FD288;
    g_ScenarioProgress = 1;
    if ((u32)(counter - 0x62) < 5u) {
      func_801BE1B0(9);
      func_8015B580(D_8014686C, 0x16);
      D_80146875 = 0x28;
    } else {
      func_801BE1B0(6);
      D_80146875 = 0x15;
    }
    break;
  }
  case 0x15: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x23u) {
      break;
    }
    *progress = 0;
    D_80146865 = 1;
    D_80146875 = 0x16;
    func_8019FA28(0x88, 0xC0000, 0x590000, 0x87);
    break;
  }
  case 0x16: {
    u8 index;

    if (g_ScenarioProgress != 10u) {
      break;
    }
    D_80146865 = 0;
    D_80146866 = 10;
    D_80146867 = 0;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    func_8015B580((u32)&D_80144F28, 0x29);
    func_8015B5A8(D_8014686C, 0x14);
    index = func_8019601C();
    if (index != 0xFF) {
      u8 *record = D_80143FC8 + (u32)index * 0x74u;

      record[0] = 1;
      record[5] = 0x77;
      record[1] = 0;
    }
    break;
  }
  case 0x19:
    D_80146875 = 0x1A;
    switch (D_80143F00) {
    case 0x85:
      func_801BE1B0(4);
      break;
    case 0x87:
      func_801BE1B0(2);
      break;
    case 0x88:
      func_801BE1B0(8);
      break;
    }
    break;
  case 0x1A: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x28u) {
      break;
    }
    *progress = 0;
    D_80146865 = 1;
    D_80146866 = 0;
    D_80146875 = 0x16;
    func_8019FA28(0x88, 0xC0000, 0x590000, 0x87);
    break;
  }
  case 0x1B:
    func_80150224(0x13);
    D_80143BB0 = 2;
    D_80146875 = 0x1C;
    break;
  case 0x1C:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 0x28: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x14u) {
      break;
    }
    *progress = 0;
    D_80146866 = 3;
    D_80146875 = 0x29;
    func_8015B580(D_8014686C, 0x26);
    switch (D_80145024 & 0x7F) {
    case 0x11:
      D_80146865 = 0;
      func_8019FA28(0x85, 0x60000, 0x5C0000, 0x85);
      break;
    case 8:
      D_80146865 = 0;
      func_8019FA28(0x85, 0x60000, 0x5C0000, 0x86);
      break;
    case 0xD:
      D_80146865 = 1;
      func_8019FA28(0x85, 0x60000, 0x5C0000, 0x86);
      break;
    case 0x10:
      D_80146865 = 2;
      func_8019FA28(0x85, 0x60000, 0x5C0000, 0x86);
      break;
    }
    break;
  }
  case 0x29: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 10u) {
      break;
    }
    D_80146875 = 0x2A;
    *progress = 0;
    D_80146866 = 0;
    func_8015B580(D_8014686C, 0x27);
    switch (D_80145024 & 0x7F) {
    case 8:
      func_8019FA28(0x85, 0x318000, 0x268000, 0x87);
      break;
    case 0x11:
      func_8019FA28(0x85, 0x318000, 0x268000, 0x88);
      break;
    }
    break;
  }
  case 0x2A:
    D_80146865 = 0;
    func_801C601C(1);
    D_80146875 = 0x2B;
    break;
  case 0x2B:
    if (g_ScenarioProgress != 5u) {
      break;
    }
    func_8014ECAC(4);
    D_80146875 = 0x2C;
    break;
  case 0x2C: {
    u16 fade;
    u8 request;

    if (D_80143C40 != 0) {
      break;
    }
    fade = D_80146258;
    request = D_80145024 & 0x7F;
    D_80146875 = 0x2D;
    g_ScenarioProgress = 0;
    D_8014832E = 0;
    D_80146258 = fade ^ 0xE;
    switch (request) {
    case 7:
      func_801C1E3C(1);
      func_801C1E3C(0);
      func_801C1DF0(6);
      break;
    case 8:
      func_801C1E3C(1);
      func_801C1DF0(7);
      break;
    case 9:
      func_801C1E3C(1);
      func_801C1DF0(6);
      break;
    case 0xA:
      func_801C1E3C(1);
      func_801C1E3C(4);
      func_801C1DF0(6);
      break;
    case 0xD:
      func_801C1E3C(0);
      func_801C1DF0(7);
      break;
    case 0xE:
      func_801C1E3C(0);
      func_801C1DF0(6);
      break;
    case 0xF:
      func_801C1E3C(0);
      func_801C1E3C(4);
      func_801C1DF0(6);
      break;
    case 0x11:
      func_801C1E3C(4);
      func_801C1DF0(7);
      break;
    case 0x12:
      func_801C1E3C(4);
      func_801C1DF0(6);
      break;
    }
    break;
  }
  case 0x2D:
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 6;
    break;
  }
}
