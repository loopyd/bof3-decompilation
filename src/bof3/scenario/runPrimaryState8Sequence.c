#include "bof3/bof3.h"

/* This target's scratchpad work-object pointer cell at 0x1F800044: the
 * sub-state 0x21 arm publishes the selected party slot's work record here and
 * then reads the record index back through it, so every access re-materialises
 * the cell address.
 * @source 0x1F800044 @kind unknown */
extern u8 *stateWorkObject;

/* Shared sub-state byte at 0x80146875: this sequence switches on it and every
 * arm that advances the sequence stores it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared primary state byte at 0x80146874: the closing tail clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared scenario progress byte at 0x80146864: most arms gate on it and two of
 * them store it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte at 0x80143BB0: the action arms arm it to 2 and
 * the two arms that follow leave the state once it reads 2.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared mode halfword at 0x80146258: sub-state 0x1F sets bit 3 and sub-state
 * 0x28 toggles it.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared gate halfword at 0x80143C40, read with lhu: sub-state 0x20 leaves the
 * state while it is non-zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared effect bank byte at 0x8014832E: sub-state 0x1F arms it to 0x1F.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared scenario flag word at 0x8014686C, read with lw: the arms at sub-states
 * 0x10 and 0x28 pass it to the shared bit-set helper.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared character bytes at 0x80146865 through 0x80146867: sub-state 0x10
 * clears all three before it resets the state.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared effect-cue byte at 0x8014933B: sub-states 0x14 and 0x1F arm it to 3
 * and sub-state 0x23 clears it.
 * @source 0x8014933B @kind unknown */
extern u8 D_8014933B;

/* Shared effect halfword at 0x8014932E: sub-state 0x26 arms it to 0x20.
 * @source 0x8014932E @kind unknown */
extern u16 D_8014932E;

/* Shared effect gate byte at 0x80149332: sub-state 0x27 leaves the state while
 * it is non-zero.
 * @source 0x80149332 @kind unknown */
extern u8 D_80149332;

/* Shared camera word pair at 0x80149308 and 0x8014930C: sub-state 0x26 copies
 * the D_80145EC4/D_80145EC8 pair into it.
 * @source 0x80149308 @kind data */
extern u32 D_80149308;
/* @source 0x8014930C @kind data */
extern u32 D_8014930C;

/* Shared source pair the sub-state 0x26 copy reads.
 * @source 0x80145EC4 @kind data */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind data */
extern u32 D_80145EC8;

/* Base of the shared per-party-slot work records at 0x80145E90; slot i sits
 * 0x140 bytes further along.
 * @source 0x80145E90 @kind unknown */
extern u8 D_80145E90[];

/* Shared party selector list at 0x80144F5A: this sequence sweeps its three
 * slots to pick the record to seed and to queue the per-member cues.
 * @source 0x80144F5A @kind unknown */
extern u8 D_80144F5A[];

/* Front-end record table base at 0x80143FC8 and the field bases of its
 * 0x74-byte rows: the original re-materialises `lui at,0x8014` for every store,
 * so each field is addressed as its own absolute symbol rather than through one
 * struct base.
 * @source 0x80143FC8 @kind unknown */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FCE @kind unknown */
extern u8 D_80143FCE[];
/* @source 0x80143FD4 @kind unknown */
extern u8 D_80143FD4[];
/* @source 0x80143FD8 @kind unknown */
extern u8 D_80143FD8[];
/* @source 0x80143FF6 @kind unknown */
extern u8 D_80143FF6[];
/* @source 0x80143FF8 @kind unknown */
extern u8 D_80143FF8[];

/* Shared slot query at 0x8019601C: it answers the next free record index, and
 * the sub-state 0x21 arm stores the answer in the work record's byte 0x0B
 * before indexing the 0x74-byte record run with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared GAME.EMI window opener at 0x801BE1B0: sub-states 0x0F, 0x14 and 0x27
 * run it with selectors 0, 2 and 4.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared front-end mode setter at 0x8014ECAC: sub-state 0x1F sets mode 1.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared scenario flag-word bit-set helper at 0x8015B580: two arms pass it the
 * shared flag word and bit 0x12 or 0x17.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared overlay release helper at 0x8015C058: the closing tail and sub-state
 * 0x10 run it before the state bytes are cleared.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end request setter at 0x8019FA28: sub-state 0x1E passes it
 * selection seed 0x77, the two 16.16 context words 0x98000 and 0x348000 and
 * request kind 0x83.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared front-end cue queue at 0x8015DF18: sub-states 0x14, 0x1F and 0x23
 * queue cues 0x206, 0x207 and 0x208 through it.
 * @source 0x8015DF18 @kind function */
extern void func_8015DF18(u16 cue);

/* Shared front-end action helper at 0x80150224: the sub-states 0 through 6 and
 * 0x19 request actions 0x21 through 0x27 and 0x2C through it, and the sub-state
 * 0x22 arm requests 0x2D through 0x30 per party member.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared two-vector camera helper at 0x801BFE34: sub-states 0x20 and 0x24 pass
 * it the context word pair and the mode 0x35.
 * @source 0x801BFE34 @kind function */
extern void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);

/* Shared camera mode applier at 0x801C7A54: sub-state 0x24 passes it the mode
 * 0x35 the previous call received.
 * @source 0x801C7A54 @kind function */
extern void func_801C7A54(s32 arg0);

/* @source 0x801FA1F0
 * @behavior Word 8 (the word stored at 0x801FE40C) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateByte (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801FA1F0 and that table word is its
 * only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the forty-one-entry absolute jump table
 * jtbl_801F6F04_main at 0x801F6F04, an overlay-local table in this payload,
 * and its 0x20-byte frame keeps $ra, $s0 and $s1. Sub-states 0 through 6
 * request front-end actions 0x23, 0x24, 0x25, 0x26, 0x27, 0x21 and 0x22
 * through the shared action helper func_80150224, arm the shared phase byte
 * D_80143BB0 (0x80143BB0) to 2 and advance the sub-state byte to 0x0A, exactly
 * as sub-state 0x19 does with action 0x2C. Sub-state 0x0A leaves the state once
 * that phase byte reads 2 and otherwise falls into the closing tail. Sub-state
 * 0x0F runs func_801BE1B0(0) and advances to 0x10. Sub-state 0x10, once the
 * shared scenario progress byte g_ScenarioProgress (0x80146864) reads 1, sets
 * scenario flag bit 0x12 of the shared flag word D_8014686C (0x8014686C, read
 * with lw) through func_8015B580, runs the shared release helper
 * func_8015C058, clears the shared character bytes D_80146865, D_80146866 and
 * D_80146867 and resets the sub-state and primary state bytes. Sub-state 0x14
 * queues cue 0x206 through func_8015DF18, arms the shared effect-cue byte
 * D_8014933B to 3, runs func_801BE1B0(2) and advances to 0x15, and sub-state
 * 0x15 resets the state once the progress byte reads 1. Sub-state 0x1E advances
 * to 0x1F and requests front selector context seed 0x77 with the context words
 * 0x98000 and 0x348000 and kind 0x83 through func_8019FA28. Sub-state 0x1F
 * sets bit 3 of the shared mode halfword D_80146258 (0x80146258, read with
 * lhu), runs func_8014ECAC(1), arms the effect bank byte D_8014832E
 * (0x8014832E) to 0x1F, queues cue 0x207, arms D_8014933B to 3 and advances to
 * 0x20. Sub-state 0x20, while the shared gate halfword D_80143C40 (0x80143C40,
 * read with lhu) is zero, runs func_801BFE34(0x118000, 0x348000, 0x35), stores
 * 2 into the progress byte and advances to 0x21. Sub-state 0x21, once the
 * progress byte reads 3, sweeps the three slots of the shared party selector
 * list D_80144F5A (0x80144F5A, read with lbu) and for each slot that reads 7
 * publishes the slot's 0x140-byte work record of the shared run at 0x80145E90
 * in the scratchpad pointer cell stateWorkObject (0x1F800044), stores the
 * record index func_8019601C answers in that record's byte 0x0B and, unless
 * that byte reads 0xFF, seeds its 0x74-byte record of the shared front-end run
 * at 0x80143FC8: byte 0x00 = 1, byte 0x05 = 6, byte 0x06 = 2, word 0x0C = 0,
 * word 0x10 = 0x24, and the halfwords at 0x2E and 0x30 copied from the work
 * record at the same offsets, then stores 4 into the progress byte; it advances
 * to 0x22. Sub-state 0x22, once the progress byte reads 6, requests action 0x30
 * for every party slot that reads 2, 0x2F for every slot that reads 4, 0x2E for
 * every slot that reads 8 and 0x2D for every slot that reads 5, then arms the
 * phase byte to 2 and advances to 0x23. Sub-state 0x23, once the progress byte
 * reads 7, clears D_8014933B, queues cue 0x208 and advances to 0x24. Sub-state
 * 0x24, once the progress byte reads 0x0A, clears that byte through one address
 * taken into a local pointer, runs func_801BFE34(0xD8000, 0x348000, 0x35) and
 * func_801C7A54(0x35) and advances to 0x25. Sub-state 0x26 copies the
 * D_80145EC4 and D_80145EC8 words into D_80149308 and D_8014930C, arms the
 * halfword D_8014932E (0x8014932E) to 0x20 and advances to 0x27. Sub-state 0x27
 * leaves the state while the byte D_80149332 (0x80149332) is non-zero, runs
 * func_801BE1B0(4) and advances to 0x28. Sub-state 0x28, once the progress byte
 * reads 1, sets scenario flag bit 0x17 of the flag word through func_8015B580,
 * toggles bit 0x08 of the mode halfword and falls into the closing tail, which
 * runs func_8015C058 and clears the sub-state byte and the primary state byte
 * D_80146874 back to zero. Every other sub-state value returns without touching
 * the tail, so the table holds forty-one entries and twenty-three of them name
 * a body. It takes no arguments, reads no other state and writes no other byte.
 * Its 0x6C8 bytes have no copy in the tree: an exact-span search over every
 * payload under out/binaries finds no second copy of the 0x6C8-byte body, and
 * the only overlay-local absolute word in it is the jump-table base 0x801F6F04,
 * which every other overlay would carry at its own address.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes: the seven record stores name their scaled offset in one
 * `u32` local that is reassigned from a fresh work-byte read before every
 * store. The inline subscript form (`D_80143FC8[(u32)work[0x0B] * 0x74u]`),
 * which is the shape the exact scena01/scena02 publish helpers use outside a
 * loop, measured 372 of 444 at 0x30 bytes over the original here: inside this
 * loop GCC hoisted the four leading table bases into $s1 through $s4 and
 * widened the frame to 0x28. Naming the offset restores the per-store
 * `lui $at,%hi(X)` fold and the original 0x20-byte frame at the original size.
 */
void runPrimaryState8Sequence(void) {
  u8 *work;
  u8 i;

  switch (D_80146875) {
  case 0:
    func_80150224(0x23);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 1:
    func_80150224(0x24);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 2:
    func_80150224(0x25);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 3:
    func_80150224(0x26);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 4:
    func_80150224(0x27);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 5:
    func_80150224(0x21);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 6:
    func_80150224(0x22);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 0x0A:
    if (D_80143BB0 == 2u) {
      return;
    }
    break;
  case 0x0F:
    func_801BE1B0(0u);
    D_80146875 = 0x10u;
    return;
  case 0x10:
    if (g_ScenarioProgress != 1u) {
      return;
    }
    func_8015B580(D_8014686C, 0x12);
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 0x14:
    func_8015DF18(0x206);
    D_8014933B = 3u;
    func_801BE1B0(2u);
    D_80146875 = 0x15u;
    return;
  case 0x15:
    if (g_ScenarioProgress != 1u) {
      return;
    }
    break;
  case 0x19:
    func_80150224(0x2C);
    D_80143BB0 = 2u;
    D_80146875 = 0x0Au;
    return;
  case 0x1E:
    D_80146875 = 0x1Fu;
    func_8019FA28(0x77, 0x98000, 0x348000, 0x83);
    return;
  case 0x1F:
    D_80146258 |= 8u;
    func_8014ECAC(1u);
    D_8014832E = 0x1Fu;
    func_8015DF18(0x207);
    D_8014933B = 3u;
    D_80146875 = 0x20u;
    return;
  case 0x20:
    if (D_80143C40 != 0u) {
      return;
    }
    func_801BFE34(0x118000, 0x348000, 0x35);
    g_ScenarioProgress = 2u;
    D_80146875 = 0x21u;
    return;
  case 0x21:
    if (g_ScenarioProgress != 3u) {
      return;
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 7u) {
        stateWorkObject = D_80145E90 + i * 0x140;
        stateWorkObject[0x0B] = func_8019601C();
        if (stateWorkObject[0x0B] != 0xFFu) {
          u32 offset;

          offset = (u32)stateWorkObject[0x0B] * 0x74u;
          D_80143FC8[offset] = 1u;
          offset = (u32)stateWorkObject[0x0B] * 0x74u;
          D_80143FCD[offset] = 6u;
          offset = (u32)stateWorkObject[0x0B] * 0x74u;
          D_80143FCE[offset] = 2u;

          work = stateWorkObject;
          offset = (u32)work[0x0B] * 0x74u;
          *(u32 *)(D_80143FD4 + offset) = 0u;
          offset = (u32)work[0x0B] * 0x74u;
          *(u32 *)(D_80143FD8 + offset) = 0x24u;
          offset = (u32)work[0x0B] * 0x74u;
          *(u16 *)(D_80143FF6 + offset) = *(u16 *)(work + 0x2E);
          offset = (u32)work[0x0B] * 0x74u;
          *(u16 *)(D_80143FF8 + offset) = *(u16 *)(work + 0x30);
          g_ScenarioProgress = 4u;
        }
      }
    }
    D_80146875 = 0x22u;
    return;
  case 0x22:
    if (g_ScenarioProgress != 6u) {
      return;
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 2u) {
        func_80150224(0x30);
      }
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 4u) {
        func_80150224(0x2F);
      }
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 8u) {
        func_80150224(0x2E);
      }
    }
    for (i = 0; i < 3; i++) {
      if (D_80144F5A[i] == 5u) {
        func_80150224(0x2D);
      }
    }
    D_80143BB0 = 2u;
    D_80146875 = 0x23u;
    return;
  case 0x23:
    if (g_ScenarioProgress != 7u) {
      return;
    }
    D_8014933B = 0u;
    func_8015DF18(0x208);
    D_80146875 = 0x24u;
    return;
  case 0x24: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x0Au) {
      return;
    }
    *progress = 0u;
    func_801BFE34(0xD8000, 0x348000, 0x35);
    func_801C7A54(0x35);
    D_80146875 = 0x25u;
    return;
  }
  case 0x26: {
    u32 pair0 = D_80145EC4;
    u32 pair1 = D_80145EC8;

    D_8014932E = 0x20u;
    D_80146875 = 0x27u;
    D_80149308 = pair0;
    D_8014930C = pair1;
    return;
  }
  case 0x27:
    if (D_80149332 != 0u) {
      return;
    }
    func_801BE1B0(4u);
    D_80146875 = 0x28u;
    return;
  case 0x28:
    if (g_ScenarioProgress != 1u) {
      return;
    }
    func_8015B580(D_8014686C, 0x17);
    D_80146258 ^= 8u;
    break;
  default:
    return;
  }

  func_8015C058();
  D_80146875 = 0u;
  D_80146874 = 0u;
}
