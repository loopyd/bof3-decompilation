#include "bof3/bof3.h"

/* Scratchpad byte at 0x1F800000: the record arm publishes the shared slot
 * query result here and then indexes the shared record run with it, so the
 * read that follows the store is a real lbu.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* Shared scenario progress byte at 0x80146864: every progress arm of this
 * sequence compares it, and two of them clear or advance it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte at 0x80143BB0: the two cue arms arm it to 2 and
 * the two arms that follow them leave the state once it reads 2.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared mode byte at 0x8014623B: sub-state 0 runs only while it does not
 * read 3.
 * @source 0x8014623B @kind unknown */
extern u8 D_8014623B;

/* Shared mode halfword at 0x80146258: sub-state 0 sets bit 3 and sub-state 8
 * toggles it.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared gate halfword at 0x80143C40: sub-states 5 and 7 wait for it to read
 * zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared effect bank byte at 0x8014832E: sub-state 5 clears it and sub-state 6
 * reloads it with 0x1F.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared cue byte at 0x80143F1F: the sub-state 5 arm stores 0x61 here.
 * @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;

/* Shared scenario flag word at 0x8014686C, read with lw: the arms at sub-state
 * 5 and 0x16 pass it to the shared bit-set helper.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared primary state byte at 0x80146874: the closing tail clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte at 0x80146875: this sequence switches on it, and every
 * arm that advances the sequence stores it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* One 0x74-byte field record of the shared main-RAM run at 0x80143FC8. The
 * record arm stores the leading active byte and the arm byte at +5, and the
 * original recomputes the 0x74-byte stride from a fresh scratchpad read for
 * each store.
 * @kind struct */
typedef struct {
  u8 unk_00;        /* 0x00 slot active byte, stored as 1 */
  u8 unk_01[4];     /* 0x01 */
  u8 unk_05;        /* 0x05 arm byte, stored as 0x9A */
  u8 unk_06[0x6E];  /* 0x06 */
} FieldRecord;

/* Base of the shared 0x74-byte field-record run at 0x80143FC8.
 * @source 0x80143FC8 @kind data */
extern FieldRecord D_80143FC8[];

/* Shared slot query at 0x8019601C: it answers the next free record index with
 * 0xFF meaning none, and the record arm publishes the answer in the scratchpad
 * byte before indexing with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared GAME.EMI helper at 0x801BE1B0: every arm that opens a window runs it
 * with the selector the original encodes as 5, 6, 8 or 9.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared overlay reset helper at 0x801C601C: the sub-state 0 arm runs it with
 * the window selector 1.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared front-end mode setter at 0x8014ECAC: two arms set mode 4 and 5.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared scenario flag-word bit-set helper at 0x8015B580: the arms pass it the
 * shared flag word and bit 0x1E or 0x2C.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared overlay release helper at 0x8015C058: the closing tail runs it before
 * clearing the sub-state and primary state bytes.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end request setter at 0x8019FA28: the sub-state 5 arm passes it
 * selection seed 0x77, the two 16.16 context words 0xA8000 and 0xB0000 and
 * request kind 0x87.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared front-end cue queue at 0x8015DF18: the sub-state 6 arm queues cue
 * 0x20C through it.
 * @source 0x8015DF18 @kind function */
extern void func_8015DF18(u16 cue);

/* Shared front-end action helper at 0x80150224: the two cue arms request
 * action 0x4B and 0x4C.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared two-vector camera helper at 0x801BFE34: the sub-state 3 arm passes it
 * the same 16.16 word twice and the mode 0x36.
 * @source 0x801BFE34 @kind function */
extern void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);

/* Shared camera mode applier at 0x801C7A54: the sub-state 3 arm passes it the
 * mode 0x36 the previous call received.
 * @source 0x801C7A54 @kind function */
extern void func_801C7A54(s32 arg0);

/* @source 0x801FA8B8
 * @behavior Word 9 (the word stored at 0x801FE410) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801FA8B8 and that table word is its
 * only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the thirty-two-entry absolute jump table
 * jtbl_801F6FAC_main at 0x801F6FAC, an overlay-local table in this payload,
 * and its 0x18-byte frame keeps only $ra. Sub-state 0, while the shared mode
 * byte D_8014623B (0x8014623B, read with lbu) does not read 3, sets bit 3 of
 * the shared mode halfword D_80146258 (0x80146258, read with lhu), runs the
 * shared window openers func_801BE1B0(5) and func_801C601C(1) and advances the
 * sub-state byte to 1. Sub-state 1, once the shared scenario progress byte
 * g_ScenarioProgress (0x80146864) reads 2, runs func_801BE1B0(6) and advances
 * to 2. Sub-state 2, once the progress byte reads 0x25, asks the shared slot
 * query func_8019601C for a record index, publishes it in the scratchpad byte
 * D_1F800000 (0x1F800000), and
 * unless that byte reads 0xFF stores the active byte 1 and the arm byte 0x9A
 * into its 0x74-byte record of the shared run at 0x80143FC8, then advances to
 * 3. Sub-state 3, once the progress byte reads 0x28, runs
 * func_801BFE34(0xE8000, 0xE8000, 0x36) and func_801C7A54(0x36) and advances
 * to 4. Sub-state 4, once the progress byte reads 0x32, clears it, runs
 * func_801BE1B0(9) and the shared front-end mode setter func_8014ECAC(4) and
 * advances to 5. Sub-state 5, while the shared gate halfword D_80143C40
 * (0x80143C40, read with lhu) reads 0, clears the effect bank byte
 * D_8014832E (0x8014832E), advances to 6, sets scenario flag bit 0x1E of the
 * shared flag word D_8014686C (0x8014686C, read with lw) through func_8015B580,
 * requests front selector context seed 0x77 with the context words 0xA8000 and
 * 0xB0000 and kind 0x87 through func_8019FA28, and stores 0x61 into the
 * shared cue byte D_80143F1F (0x80143F1F). Sub-state 6, once the progress byte
 * reads 1, runs func_8014ECAC(5), arms the effect bank byte to 0x1F, queues
 * cue 0x20C through func_8015DF18 and advances to 7. Sub-state 7, while that
 * same gate halfword reads 0, stores 2 into the progress byte and advances to
 * 8. Sub-state 8, once the progress byte reads 0xE, toggles bit 0x08 of the
 * mode halfword and falls into the closing tail. Sub-state 0x14 requests
 * front-end action 0x4B through func_80150224, stores 2 into the shared phase
 * byte D_80143BB0 (0x80143BB0) and advances to 0x15. Sub-state 0x15 leaves the
 * state while that phase byte reads 2, runs func_801BE1B0(8) and advances to
 * 0x16 once the progress byte reads 0xA, and otherwise falls into the closing
 * tail. Sub-state 0x16, once the progress byte reads 0x19, sets scenario flag
 * bit 0x2C of the flag word through func_8015B580 and falls into that tail.
 * Sub-state 0x1E requests action 0x4C, stores 2 into the phase byte and
 * advances to 0x1F, and sub-state 0x1F leaves the state while that phase byte
 * reads 2 and otherwise falls into the tail, which runs the shared release
 * helper func_8015C058 and clears the sub-state byte and the primary state
 * byte D_80146874 back to zero. Sub-states 9 through 0x13, 0x17 through 0x1D
 * and every value from 0x20 up return without touching the tail, so the table
 * holds thirty-two entries and only fourteen of them name a body. It takes no
 * arguments, reads no other state and writes no other byte. Its 0x3D0 bytes
 * have no copy in the tree: an exact-span search over every payload under
 * out/binaries finds no second copy of the 0x3D0-byte body, and the only
 * overlay-local absolute word in it is the jump-table base 0x801F6FAC, which
 * every other overlay would carry at its own address.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes: the 0x74-byte record stride, the scratchpad publish
 * and the two field stores come from the exact sibling
 * stepSubstatesThroughFrontEndContextsAndFieldRecordSlotsThenReleaseStateScenarioScena1200_801F9F0C
 * (0x801F9F0C), whose record arm emits the same masked 0xFF test and the same
 * per-store scratchpad reload. The single closing tail after the switch,
 * reached by four break sites, comes from this target's exact sibling
 * armProgress0UnlessMode2ThenSetFlagBit13OrSeedFrontSelectorContext76AndClearStateScenarioScena0900_801FA08C
 * (0x801FA08C). Both seeds matched at their first gate, so no losing spelling
 * is recorded.
 */
void runPrimaryState9SubstateSequenceScenarioScena0900_801FA8B8(void) {
  switch (D_80146875) {
  case 0:
    if (D_8014623B == 3u) {
      return;
    }
    D_80146258 |= 8u;
    func_801BE1B0(5u);
    func_801C601C(1u);
    D_80146875 = 1u;
    return;
  case 1:
    if (g_ScenarioProgress != 2u) {
      return;
    }
    func_801BE1B0(6u);
    D_80146875 = 2u;
    return;
  case 2:
    if (g_ScenarioProgress != 0x25u) {
      return;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) != 0xFF) {
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x9A;
    }
    D_80146875 = 3u;
    return;
  case 3:
    if (g_ScenarioProgress != 0x28u) {
      return;
    }
    func_801BFE34(0xE8000, 0xE8000, 0x36);
    func_801C7A54(0x36);
    D_80146875 = 4u;
    return;
  case 4: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x32u) {
      return;
    }
    *progress = 0u;
    func_801BE1B0(9u);
    func_8014ECAC(4u);
    D_80146875 = 5u;
    return;
  }
  case 5:
    if (D_80143C40 != 0u) {
      return;
    }
    D_8014832E = 0u;
    D_80146875 = 6u;
    func_8015B580(D_8014686C, 0x1E);
    func_8019FA28(0x77, 0xA8000, 0xB0000, 0x87);
    D_80143F1F = 0x61;
    return;
  case 6:
    if (g_ScenarioProgress != 1u) {
      return;
    }
    func_8014ECAC(5u);
    D_8014832E = 0x1Fu;
    func_8015DF18(0x20C);
    D_80146875 = 7u;
    return;
  case 7:
    if (D_80143C40 != 0u) {
      return;
    }
    g_ScenarioProgress = 2u;
    D_80146875 = 8u;
    return;
  case 8:
    if (g_ScenarioProgress != 0xEu) {
      return;
    }
    D_80146258 ^= 8u;
    break;
  case 0x14:
    func_80150224(0x4B);
    D_80143BB0 = 2u;
    D_80146875 = 0x15u;
    return;
  case 0x15:
    if (D_80143BB0 == 2u) {
      return;
    }
    if (g_ScenarioProgress != 0xAu) {
      break;
    }
    func_801BE1B0(8u);
    D_80146875 = 0x16u;
    return;
  case 0x16:
    if (g_ScenarioProgress != 0x19u) {
      return;
    }
    func_8015B580(D_8014686C, 0x2C);
    break;
  case 0x1E:
    func_80150224(0x4C);
    D_80143BB0 = 2u;
    D_80146875 = 0x1Fu;
    return;
  case 0x1F:
    if (D_80143BB0 == 2u) {
      return;
    }
    break;
  default:
    return;
  }

  func_8015C058();
  D_80146875 = 0u;
  D_80146874 = 0u;
}
