#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: every waiting arm of
 * this body compares it against a constant and the two advancing arms add one
 * to it. */
extern u8 g_ScenarioProgress;
/* Shared front-end mode byte of the main executable: mode 2 is busy and stops
 * sub-state 0 from acting. */
extern u8 D_80143BB0;
/* Shared scenario flag word of the main executable, passed whole to the shared
 * flag-bit setter. */
extern u32 D_8014686C;
/* Shared mode halfword of the main executable: the closing arm clears its
 * 0x80 bit. */
extern u16 D_80146258;
/* Shared front-end allocator answer byte: the two claim arms publish the index
 * func_8019601C answers there and re-read it for every record store, because a
 * store through the record table may alias the slot itself. */
extern u8 D_1F800000[];
/* Signed shared byte cell the second claim arm publishes the allocator answer
 * into, and re-reads for every record store of its own. */
extern s8 D_80146873[];
extern u8 D_80146874;
extern u8 D_80146875;
/* Shared halfword countdown the two countdown arms decrement, and that
 * sub-state 9 re-arms with 10. */
extern u16 D_80146876;
/* Shared effect bank byte and the two tint channel bytes the sequence arms
 * seed. */
extern u8 D_8014832E;
extern u8 D_80143F1D;
extern u8 D_80143F1E;
/* Shared coordinate halfwords the claim arms copy into the seeded record. */
extern s16 D_801492DA;
extern s16 D_801492DC;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses every field through its own
 * absolute symbol, because the original re-forms the base and adds the scaled
 * row offset for each access.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 5, the entry handler selector seeded with 0x13 by the two scratchpad
 * claim arms and with 0x22 by the signed-byte claim arm.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row byte 9, the entry state byte seeded with 0x38 by the two scratchpad claim
 * arms.
 * @source 0x80143FD1 @kind unknown
 */
extern u8 D_80143FD1[];
/* Row word 0x34, the first coordinate of the seeded position pair.
 * @source 0x80143FFC @kind unknown
 */
extern u8 D_80143FFC[];
/* Row word 0x38, the second coordinate of the seeded position pair.
 * @source 0x80144000 @kind unknown
 */
extern u8 D_80144000[];
/* Row word 0x3C, the third coordinate of the seeded position pair.
 * @source 0x80144004 @kind unknown
 */
extern u8 D_80144004[];
/* Row word 0x64, the fourth word of the seeded row, which the scratchpad claim
 * arms fill with a negative constant.
 * @source 0x8014402C @kind unknown
 */
extern u8 D_8014402C[];
/* Row word 0x68, the fifth word of the seeded row.
 * @source 0x80144030 @kind unknown
 */
extern u8 D_80144030[];
/* Row word 0x6C, the sixth word of the seeded row.
 * @source 0x80144034 @kind unknown
 */
extern u8 D_80144034[];

/* Front-end record allocator of the shared main-executable data segment: it
 * answers the index of the next free 0x74-byte row at 0x80143FC8, or 0xFF when
 * every row is occupied. Splat binds its address through this target's
 * generated undefined_funcs_auto.txt. */
u8 func_8019601C(void);
/* Shared front selector request setter at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared cue-and-mode helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801FB268
 * @behavior Word 7 (the word stored at 0x801FCFFC) of the nine-word primary
 * state handler table at 0x801FCFE0 that
 * dispatchPrimaryStateHandlerScenarioScena0300_801F92F4 (0x801F92F4) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets this address and that table word is its
 * only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the fourteen-entry absolute jump table
 * jtbl_801F6DE4_main at 0x801F6DE4 in this payload leading table data, bounded
 * by sltiu against 0xE, and any other value returns without a store. Every
 * shared cell is written through its own canonical label, so the original
 * re-forms the shared base for each store and no local pointer survives across
 * a call.
 * Sub-state 0 refuses to act while the shared front-end mode byte D_80143BB0
 * reads 2; otherwise it decrements the shared halfword countdown D_80146876
 * whenever that countdown is non-zero, and once the countdown reads zero it
 * clears the shared progress byte g_ScenarioProgress, runs func_801BE1B0(4) and
 * advances the sub-state byte to 1. Sub-state 1 waits until that progress byte
 * reads 2, then clears the shared effect bank byte D_8014832E and advances to
 * 2. Sub-state 2 requests front selector context seed 0x63 with the context
 * words 0x68000 and 0x230000 and kind 1 through func_8019FA28, stores 0xFF in
 * the two tint channel bytes D_80143F1D and D_80143F1E and advances to 3.
 * Sub-state 3 arms the effect bank byte to 0x1F and advances to 4. Sub-state 4
 * waits for progress 7 and then requests seed 0x63 with the context words
 * 0x230000 and 0x510000 and kind 0x85 through func_8019FA28, advancing to 5.
 * Sub-state 5, once progress reads 0x12, asks the shared allocator
 * func_8019601C for a front-end record index, publishes that index in
 * scratchpad byte 0 and, unless the byte reads 0xFF, seeds the 0x74-byte record
 * it selects: byte 0 = 1, byte 5 = 0x13, word 0x64 = -0x32E, word 0x68 and word
 * 0x6C = the shared coordinate halfwords D_801492DA and D_801492DC, and byte 9
 * = 0x38, every store after the first re-reading the index from scratchpad byte
 * 0. It then advances to 6 on both paths. Sub-state 6, once progress reads
 * 0x16, asks the allocator again and publishes the answer in the signed shared
 * byte D_80146873; unless that byte reads -1 it seeds the selected record with
 * byte 0 = 1, byte 5 = 0x22, word 0x34 = 0x230000, word 0x38 = 0x4F0000 and
 * word 0x3C = 0x4800000, re-reading the signed index for every store after the
 * first, and then advances to 7 on both paths. Sub-state 7 repeats sub-state 5
 * once progress reads 0x18, with word 0x64 = -0x2AA, and advances to 8.
 * Sub-state 8 waits for progress 0x27, clears the effect bank byte and advances
 * to 9. Sub-state 9 re-arms the countdown D_80146876 with 10, advances the
 * sub-state byte to 10 and adds one to the progress byte. Sub-state 10
 * decrements the countdown while it is non-zero and otherwise arms the effect
 * bank byte to 0x1F, advances to 11 and adds one to the progress byte.
 * Sub-state 11 waits for progress 0x2F and advances to 12. Sub-state 12
 * advances to 13 without touching any other cell. Sub-state 13 waits for
 * progress 0x34, runs the shared overlay release helper func_8015C058, sets
 * scenario flag bit 0x1D of the shared flag word D_8014686C through
 * func_8015B580, clears the progress byte, the sub-state byte and the primary
 * state byte D_80146874, and clears bit 0x80 of the shared mode halfword
 * D_80146258.
 * It takes no arguments and returns nothing; its 0x18-byte frame keeps $ra and
 * one saved register, which sub-state 13 uses for the progress byte address it
 * holds across the two calls. The countdown store of sub-states 0 and 10 is one
 * shared tail, and so are the effect-bank clear plus sub-state store of
 * sub-states 1 and 8 and the sub-state store plus progress increment of
 * sub-states 9 and 10. Its 0x63C bytes have no copy in the tree: an exact-span
 * search over every payload under out/binaries finds no second copy of the
 * body, and an opcode-and-register-field pass over the same corpus finds no
 * member that differs in immediates alone, so the clone lever is declined and
 * every literal here was written from this span's own bytes. The nearest shape
 * donor is the sibling runPrimaryState4SubstateSequenceScenarioScena1100_801F7F08
 * (0x801F7F08), whose own gate measured exact and whose scratchpad claim arm
 * supplied the statement order and operand spelling of sub-states 5 and 7; the
 * claim arm of the scena00 lift func_801FB204 (0x801FB204) supplied the
 * substate-value and allocator-answer temporaries.
 * @status partial
 * @match 99.25
 * @residual Three of the 399 instructions differ by scheduler order alone,
 * with no instruction added, dropped or replaced: sub-state 9 loads its
 * countdown constant 0x10 after the progress pointer materialisation where the
 * original loads it first (first difference at original offset 0x528), and the
 * closing arm reads the shared mode halfword above the progress byte clear, so
 * that clear sits one slot later than the original instead of in the second
 * call's delay slot. Every literal, cell and callee binding measured equal, and
 * the fourteen-word jump table emitted into .rodata measures byte-identical to
 * jtbl_801F6DE4_main.
 */
void runPrimaryState7SubstateSequenceScenarioScena0300_801FB268(void) {
  u8 *slot;
  u8 index;
  s8 *signedSlot;
  s8 signedIndex;
  u16 *countdown;
  u8 *progress;
  u8 *closingProgress;
  u8 next;
  u8 value;

  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2u) {
      break;
    }
    countdown = &D_80146876;
    if (*countdown != 0u) {
      *countdown = *countdown - 1;
      break;
    }
    g_ScenarioProgress = 0;
    func_801BE1B0(4);
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress != 2u) {
      break;
    }
    D_8014832E = 0;
    D_80146875 = 2;
    break;
  case 2:
    func_8019FA28(0x63, 0x68000, 0x230000, 1);
    D_80143F1D = 0xFF;
    D_80143F1E = 0xFF;
    D_80146875 = 3;
    break;
  case 3:
    D_8014832E = 0x1F;
    D_80146875 = 4;
    break;
  case 4:
    if (g_ScenarioProgress != 7u) {
      break;
    }
    func_8019FA28(0x63, 0x230000, 0x510000, 0x85);
    D_80146875 = 5;
    break;
  case 5:
    if (g_ScenarioProgress != 0x12u) {
      break;
    }
    index = func_8019601C();
    slot = D_1F800000;
    slot[0] = index;
    if (index != 0xFFu) {
      D_80143FC8[(u32)index * 0x74u] = 1;
      D_80143FCD[(u32)slot[0] * 0x74u] = 0x13;
      *(s32 *)(D_8014402C + (u32)slot[0] * 0x74u) = -0x32E;
      *(s32 *)(D_80144030 + (u32)slot[0] * 0x74u) = D_801492DA;
      *(s32 *)(D_80144034 + (u32)slot[0] * 0x74u) = D_801492DC;
      D_80143FD1[(u32)slot[0] * 0x74u] = 0x38;
    }
    D_80146875 = 6;
    break;
  case 6:
    if (g_ScenarioProgress != 0x16u) {
      break;
    }
    signedIndex = func_8019601C();
    signedSlot = D_80146873;
    signedSlot[0] = signedIndex;
    if (signedIndex != -1) {
      D_80143FC8[(u32)signedIndex * 0x74u] = 1;
      D_80143FCD[(u32)signedSlot[0] * 0x74u] = 0x22;
      *(u32 *)(D_80143FFC + (u32)signedSlot[0] * 0x74u) = 0x230000;
      *(u32 *)(D_80144000 + (u32)signedSlot[0] * 0x74u) = 0x4F0000;
      *(u32 *)(D_80144004 + (u32)signedSlot[0] * 0x74u) = 0x4800000;
    }
    D_80146875 = 7;
    break;
  case 7:
    if (g_ScenarioProgress != 0x18u) {
      break;
    }
    index = func_8019601C();
    slot = D_1F800000;
    slot[0] = index;
    if (index != 0xFFu) {
      D_80143FC8[(u32)index * 0x74u] = 1;
      D_80143FCD[(u32)slot[0] * 0x74u] = 0x13;
      *(s32 *)(D_8014402C + (u32)slot[0] * 0x74u) = -0x2AA;
      *(s32 *)(D_80144030 + (u32)slot[0] * 0x74u) = D_801492DA;
      *(s32 *)(D_80144034 + (u32)slot[0] * 0x74u) = D_801492DC;
      D_80143FD1[(u32)slot[0] * 0x74u] = 0x38;
    }
    D_80146875 = 8;
    break;
  case 8:
    if (g_ScenarioProgress != 0x27u) {
      break;
    }
    D_8014832E = 0;
    D_80146875 = 9;
    break;
  case 9:
    value = 10;
    D_80146876 = value;
    progress = &g_ScenarioProgress;
    value = *progress + 1;
    next = 10;
    D_80146875 = next;
    *progress = value;
    break;
  case 10:
    countdown = &D_80146876;
    if (*countdown != 0u) {
      *countdown = *countdown - 1;
      break;
    }
    value = 0x1F;
    D_8014832E = value;
    progress = &g_ScenarioProgress;
    value = *progress + 1;
    next = 11;
    D_80146875 = next;
    *progress = value;
    break;
  case 11:
    if (g_ScenarioProgress != 0x2Fu) {
      break;
    }
    D_80146875 = 12;
    break;
  case 12:
    D_80146875 = 13;
    break;
  case 13:
    closingProgress = &g_ScenarioProgress;
    if (*closingProgress != 0x34u) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 0x1D);
    *closingProgress = 0;
    D_80146874 = 0;
    D_80146875 = 0;
    D_80146258 = D_80146258 & 0xFF7F;
    break;
  }
}
