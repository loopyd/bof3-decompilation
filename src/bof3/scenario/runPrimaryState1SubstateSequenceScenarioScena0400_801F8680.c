#include "bof3/bof3.h"

/* Shared scenario progress byte of the main-executable data segment, owned by
 * the shared symbol map. Substates 2, 3, 6, 7, 13, 14, 17, 19, 21, 22, 23 and
 * 24 gate on it, substates 5, 6, 13, 16, 20, 25 and 26 advance it, and
 * substates 18, 20, 21, 22, 23 and 30 clear it.
 * @source 0x80146864 @kind unknown
 */
extern u8 g_ScenarioProgress;

/* Second byte of the same shared word: the request hand-off byte that
 * substates 20, 21, 22 and 23 write.
 * @source 0x80146865 @kind unknown
 */
extern u8 D_80146865;

/* Signed primary-state byte of the shared state tuple; this overlay's
 * dispatchPrimaryStateHandlerScenarioScena0400_801F863C indexes the handler run
 * at 0x801FA210 with it, and the exit arms of this body store 0 into it to
 * leave primary state 1 for the state-0 noop handler.
 * @source 0x80146874 @kind unknown
 */
extern s8 D_80146874;

/* Substate byte of the same shared tuple. This body is the state-1 controller
 * keyed on it.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

/* Shared countdown halfword of the same tuple, waited out by substates 1, 3, 5,
 * 6, 9, 12, 18 and 20 and armed by substates 4, 5, 11, 17 and 19.
 * @source 0x80146876 @kind unknown
 */
extern u16 D_80146876;

/* Signed allocator answer byte: substate 3 publishes the object index that
 * func_8019601C returns into it, and substates 3 and 4 index the shared record
 * table with it through a signed load.
 * @source 0x80146873 @kind unknown
 */
extern s8 D_80146873;

/* Shared scenario mode word; substate 0 clears its bit 8, substate 19 raises
 * its bit 7.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Shared scenario bank word that substate 20 commits a stage against.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

/* Shared area-mode byte that substates 10, 13 and 28 test against 2 and
 * substates 9 and 12 set to 2.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared front-end cue selector byte read by the closing substate 30.
 * @source 0x80145029 @kind unknown
 */
extern u8 D_80145029;

/* Effect-bank selector byte, armed to 0x1F by substates 13 and 16 and cleared
 * by substates 11, 15, 17 and 19.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;

/* First word of the shared render pair that substate 15 writes.
 * @source 0x80149308 @kind unknown
 */
extern u32 D_80149308;
/* Second word of the same pair.
 * @source 0x8014930C @kind unknown
 */
extern u32 D_8014930C;

/* Shared 16-bit rate word: substate 8 derives it from the countdown and
 * substate 13 clears it.
 * @source 0x8014932C @kind unknown
 */
extern u16 D_8014932C;

/* Shared step word that substate 6 advances by the signed scale table entry of
 * the countdown value it spent.
 * @source 0x80149318 @kind unknown
 */
extern u32 D_80149318;

/* Shared mode byte of the same block, set to 2 by substates 6, 8 and 13.
 * @source 0x80149333 @kind unknown
 */
extern u8 D_80149333;

/* Row byte 0 of the shared 0x74-byte record table at 0x80143FC8: the occupancy
 * flag substate 3 sets to 1 and substate 4 tests bit 0 of.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 5, the entry handler selector that substate 3 seeds with 0x29.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];

/* Signed scale table indexed by the substate-6 countdown masked to three bits:
 * each entry is shifted left by 6 and added to D_80149318.
 * @source 0x801FA21C @kind table
 */
extern s8 D_801FA21C[];

/* Shared halfword that substates 11 and 15 wait to fall to zero.
 * @source 0x80143C40 @kind unknown
 */
extern u16 D_80143C40;

/* Shared stage halfword that substate 28 arms to 7 and substate 29 tests
 * against 2.
 * @source 0x80143B90 @kind unknown
 */
extern u16 D_80143B90;

/* Shared record flag bytes written by substates 18, 20 and 28.
 * @source 0x80143F1E @kind unknown
 */
extern u8 D_80143F1E;
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;

/* Shared front-end cells that substate 28 seeds.
 * @source 0x80148650 @kind unknown
 */
extern u8 D_80148650;
/* @source 0x8014865C @kind unknown */
extern s8 D_8014865C;
/* @source 0x80148698 @kind unknown */
extern u8 D_80148698;
/* @source 0x80148699 @kind unknown */
extern u8 D_80148699;

void func_80150224(s32 arg0);
void func_80153DE8(void);
void func_8015507C(s16 x, s16 y);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015D404(u32 arg0, s32 arg1);
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void func_8015DF18(u16 arg0);
void func_801636A0(s32 arg0, s32 arg1);
u32  func_80163EA0(void);
s32  func_8019601C(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C16B8(void);

/* @source 0x801F8680
 * @behavior The overlay handler of primary state 1: word 1 of the primary-state
 *           handler run at 0x801FA210, which
 *           dispatchPrimaryStateHandlerScenarioScena0400_801F863C indexes with
 *           the signed shared state byte D_80146874. Word 0 of that run is the
 *           inert noopHandlerScenarioScena0400_801F8678 and word 2 is the
 *           state-2 substate controller at 0x801F8F98, so this body is the
 *           overlay's second primary state and its only non-inert one besides
 *           state 2. It switches on the shared substate byte D_80146875 with an
 *           unsigned bound of 31, so values 0..30 select a case, substate 27 has
 *           no arm and falls straight to the exit, and the 31-word dispatch
 *           table the original placed in the payload's leading table data at
 *           0x801F6C04 is this target's manifest section placement. Substate 0
 *           leaves primary state 1 - it stores 0 into D_80146874 and D_80146875
 *           and clears bit 8 of D_80146258 - unless the area-mode byte
 *           D_80143BB0 already reads 2. Substates 1, 5, 9, 12, 18 and 20 wait
 *           out the shared countdown D_80146876 and decrement it while it is
 *           non-zero; substate 1 then arms a scene change through
 *           func_801BE1B0(0); substate 5 calls func_8015DF18(0x200), arms the
 *           countdown to 0x3C and increments the progress byte; substates 9 and
 *           12 request func_80150224(0xD) and func_80150224(0xE) and set the
 *           area-mode byte D_80143BB0 to 2; substate 18 clears the progress
 *           byte, requests front context (0x2A, 0x1E0000, 0x240000, 3) through
 *           func_8019FA28 and arms D_80143F1F to 0x54 and D_80143F1E to 0xFF;
 *           substate 20 clears D_80146865 and the progress byte, commits stage 4
 *           through func_8015B580(D_8014686C, 4), requests context (0x28,
 *           0xAC0000, 0x210000, 0x82) and arms D_80143F1E to 0xFF. Substate 6
 *           spends the countdown one tick per frame: while it is non-zero the
 *           body decrements it, sets the mode byte D_80149333 to 2 and advances
 *           the shared step word D_80149318 by the signed scale table entry
 *           D_801FA21C[count & 3] shifted left by 6, and a spent countdown instead
 *           releases the front selector through func_8015C058, resets the
 *           progress byte, D_80146874 and D_80146875 and increments the progress
 *           byte. Substates 2, 3, 7, 14, 17, 19, 21, 22, 23 and 24 are gated on
 *           the progress byte: substate 2 waits for 1 and requests
 *           func_8015507C(0x57, 0x12) and func_8015DF18(0x103); substate 3 waits
 *           for 2, claims one 0x74-byte row of the shared record table at
 *           0x80143FC8 from the allocator func_8019601C, publishes that answer in
 *           D_80146873 and, unless the signed answer is -1, sets the row's
 *           occupancy byte to 1 and its entry selector (row byte 5) to 0x29;
 *           substate 4 waits until that occupancy byte carries bit 0 and then
 *           arms the countdown to 0x1E; substate 7 waits for 0x14 and requests
 *           func_8015D4F8(0, 1, 0x64, 0x40) and func_801636A0(4, 1); substate 14
 *           waits for 0x23 and calls func_8014ECAC(0); substate 17 waits for
 *           0x2B, arms the countdown to 0xA and clears D_8014832E; substate 19
 *           waits for 1, arms the countdown to 0xA, clears D_8014832E and raises
 *           bit 7 of D_80146258; substates 21, 22 and 23 walk the progress byte
 *           through 5, 2 and 1, clearing it each time, write D_80146865 as 1, 2
 *           and 0 and issue the context requests (0x28, 0x6C0000, 0x70000, 0),
 *           (0x28, 0x420000, 0x60000, 0) and (0x28, 0xAC0000, 0x210000, 0x83);
 *           substate 24 waits for 9 and requests func_8015D4F8(0, 3, 0x64, 0x10)
 *           and func_801636A0(3, 1). Substates 10 and 13 advance once the
 *           area-mode byte D_80143BB0 stops reading 2; substate 13 also
 *           increments the progress byte, requests func_80161C20(0x3F, 0x64, 8),
 *           arms D_8014832E to 0x1F, clears D_8014932C, sets D_80149333 to 2 and
 *           calls func_8014ECAC(1). Substate 11 waits for zero at D_80143C40,
 *           arms the countdown to 0x3C and clears D_8014832E, while substate 15
 *           waits on the same halfword and then runs func_801C16B8,
 *           func_8015D4F8(0, 1, 0x64, 0x10) and func_801636A0(0, 1), writes the
 *           render pair D_80149308/D_8014930C with 0xAC0000 and 0x210000, clears
 *           D_8014832E and resets the overlay through func_80153DE8. Substate 8
 *           reads the countdown as a rate: while it does not read 0x80 the body
 *           stores nine times it into D_8014932C, sets D_80149333 to 2 and
 *           increments it, and at 0x80 it requests func_8015D404(0, 1) and halves
 *           the countdown. Substate 16 waits for a non-zero func_80163EA0(),
 *           requests func_8015D404(0, 1) and func_80161C20(0x30, 0x64, 8), arms
 *           D_8014832E to 0x1F and increments the progress byte, while
 *           substates 25 and 26 wait on func_80163EA0() and the loader poll
 *           func_80162D00 and, when either answers non-zero, increment the
 *           progress byte and hand over to substate 27, which has no arm.
 *           Substate 28 waits for D_80143BB0 to differ from 2 and then writes
 *           the front-end cells D_8014865C = -2, D_80148698 = 1 and D_80143B90 =
 *           7 and clears D_80148650 and D_80148699. Substate 29 waits for
 *           D_80143B90 to read 2 and shares the closing arm of substate 30, which
 *           requests the cue func_80161C20(D_80145029, 0x64, 8), clears the
 *           progress byte, releases the front selector with func_8015C058 and
 *           resets both D_80146874 and D_80146875 to 0. The body takes no
 *           arguments, returns nothing, and its 0x18-byte frame holds $ra over
 *           $s0, which keeps the address of D_80146876 across the calls of
 *           substates 5 and 8.
 *
 * Shape notes from the original bytes: this body was seeded on the exact
 * sibling runPrimaryState2SubstateSequenceScenarioScena0400_801F8F98, the word-2
 * handler of the same 0x801FA210 run, which carries the same dispatch prologue -
 * the substate byte read hoisted above the frame setup, the unsigned 31 bound,
 * the scaled table read and the indirect jump - plus the same countdown and
 * progress-increment tails and the same allocator-claim block; each arm here
 * carries its own substates, literals and callees. The compiler merges the
 * effect-bank clear and the substate store of substates 11 and 17 into one tail
 * block and the non-zero test of substates 25 and 26 into another, which is why
 * those pairs share their last two stores. Substate 3 keeps the allocator answer
 * in the signed byte cell D_80146873 and reads it back with a signed load for
 * the second table index, and substate 4 reads the same cell signed, so both
 * index the shared record table with a sign-extended byte. Substate 6 indexes
 * D_801FA21C with the countdown masked to three bits and posts the decremented
 * value to the counter, and substate 8 posts the incremented value; naming each
 * of those two later values in its own wide local moved the count/count-minus-one
 * and count/masked count register pairs onto the original's registers (572 and
 * then 581 of 582 instructions), while the same statements spelled as one
 * expression left them exchanged at 568. A scan of the 396 extracted EMI
 * payloads under out/binaries found no payload holding this 0x918 span, so the
 * clone lever was declined and every literal here was written from this span's
 * own bytes.
 * @status partial
 * @match 99.83
 * @residual The substate-6 arm emits the counter store sh a0,0(a1) one word
 *           before the step-word address build and load where the original emits
 *           it after them; every register, the 0x918-byte size and the 31-word
 *           dispatch table at 0x801F6C04 already match.
 */
void runPrimaryState1SubstateSequenceScenarioScena0400_801F8680(void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2) {
        break;
      }

      D_80146874 = 0;
      D_80146875 = 0;
      D_80146258 = D_80146258 & 0xfeffu;
      break;

    case 1: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      func_801BE1B0(0u);
      D_80146875 = 2u;
      break;
    }

    case 2:
      if (g_ScenarioProgress != 1u) {
        break;
      }

      func_8015507C(0x57, 0x12);
      func_8015DF18(0x103u);
      D_80146875 = 3u;
      break;

    case 3: {
      s8* scratch;
      s8  object_index;

      if (g_ScenarioProgress != 2u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_80146873;
      *scratch = object_index;

      if (object_index != -1) {
        D_80143FC8[(s32)object_index * 0x74] = 1u;
        D_80143FCD[(s32)*scratch * 0x74] = 0x29u;
      }

      D_80146875 = 4u;
      break;
    }

    case 4:
      if ((D_80143FC8[(s32)D_80146873 * 0x74] & 1u) != 0) {
        break;
      }

      D_80146876 = 0x1eu;
      D_80146875 = 5u;
      break;

    case 5: {
      u16* counter = &D_80146876;
      u8*  progress;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      func_8015DF18(0x200u);
      progress = &g_ScenarioProgress;
      *counter = 0x3cu;
      D_80146875 = 6u;
      *progress = *progress + 1;
      break;
    }

    case 6: {
      u16* counter = &D_80146876;
      u16  count;
      u8*  progress;

      count = *counter;

      if (count != 0) {
        u16 next = count - 1;

        D_80149318 = D_80149318 + ((s32)D_801FA21C[count & 3] << 6);
        D_80149333 = 2u;
        *counter = next;
        break;
      }

      func_8015C058();
      progress = &g_ScenarioProgress;
      D_80146874 = 0;
      D_80146875 = 0;
      *progress = *progress + 1;
      break;
    }

    case 7:
      if (g_ScenarioProgress != 0x14u) {
        break;
      }

      func_8015D4F8(0u, 1u, 0x64, 0x40);
      func_801636A0(4, 1);
      D_80146875 = 8u;
      break;

    case 8: {
      u16* counter = &D_80146876;
      u16  count;

      count = *counter;

      if (count != 0x80u) {
        u16 next = count + 1;

        D_8014932C = count * 9;
        D_80149333 = 2u;
        *counter = next;
        break;
      }

      func_8015D404(0u, 1);
      *counter = *counter >> 1;
      D_80146875 = 9u;
      break;
    }

    case 9: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      func_80150224(0xd);
      D_80143BB0 = 2u;
      D_80146875 = 0xau;
      break;
    }

    case 10:
      if (D_80143BB0 == 2) {
        break;
      }

      D_80146875 = 0xbu;
      break;

    case 11:
      if (D_80143C40 != 0) {
        break;
      }

      D_80146876 = 0x3cu;
      D_8014832E = 0;
      D_80146875 = 0xcu;
      break;

    case 12: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      func_80150224(0xe);
      D_80143BB0 = 2u;
      D_80146875 = 0xdu;
      break;
    }

    case 13: {
      u8* progress;

      if (D_80143BB0 == 2) {
        break;
      }

      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      func_80161C20(0x3f, 0x64, 8);
      D_8014832E = 0x1fu;
      D_8014932C = 0u;
      D_80149333 = 2u;
      func_8014ECAC(1u);
      D_80146875 = 0xeu;
      break;
    }

    case 14:
      if (g_ScenarioProgress != 0x23u) {
        break;
      }

      func_8014ECAC(0u);
      D_80146875 = 0xfu;
      break;

    case 15:
      if (D_80143C40 != 0) {
        break;
      }

      func_801C16B8();
      func_8015D4F8(0u, 1u, 0x64, 0x10);
      func_801636A0(0, 1);
      D_80149308 = 0xac0000u;
      D_8014832E = 0;
      D_8014930C = 0x210000u;
      func_80153DE8();
      D_80146875 = 0x10u;
      break;

    case 16: {
      u8* progress;

      if ((u8)func_80163EA0() == 0) {
        break;
      }

      func_8015D404(0u, 1);
      func_80161C20(0x30, 0x64, 8);
      D_8014832E = 0x1fu;
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
      func_8014ECAC(1u);
      D_80146875 = 0x11u;
      break;
    }

    case 17:
      if (g_ScenarioProgress != 0x2bu) {
        break;
      }

      D_80146876 = 0xau;
      D_8014832E = 0;
      D_80146875 = 0x12u;
      break;

    case 18: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      g_ScenarioProgress = 0;
      func_8019FA28(0x2au, 0x1e0000u, 0x240000u, 3u);
      D_80143F1F = 0x54u;
      D_80143F1E = 0xffu;
      D_80146875 = 0x13u;
      break;
    }

    case 19:
      if (g_ScenarioProgress != 1u) {
        break;
      }

      D_80146876 = 0xau;
      D_8014832E = 0;
      D_80146875 = 0x14u;
      D_80146258 = D_80146258 | 0x80u;
      break;

    case 20: {
      u16* counter = &D_80146876;

      if (*counter != 0) {
        *counter = *counter - 1;
        break;
      }

      D_80146865 = 0;
      g_ScenarioProgress = 0;
      func_8015B580(D_8014686C, 4);
      func_8019FA28(0x28u, 0xac0000u, 0x210000u, 0x82u);
      D_80143F1E = 0xffu;
      D_80146875 = 0x15u;
      break;
    }

    case 21: {
      u8* progress = &g_ScenarioProgress;

      if (*progress != 5u) {
        break;
      }

      *progress = 0;
      D_80146865 = 1u;
      func_8019FA28(0x28u, 0x6c0000u, 0x70000u, 0u);
      D_80146875 = 0x16u;
      break;
    }

    case 22: {
      u8* progress = &g_ScenarioProgress;

      if (*progress != 2u) {
        break;
      }

      *progress = 0;
      D_80146865 = 2u;
      func_8019FA28(0x28u, 0x420000u, 0x60000u, 0u);
      D_80146875 = 0x17u;
      break;
    }

    case 23: {
      u8* progress = &g_ScenarioProgress;

      if (*progress != 1u) {
        break;
      }

      D_80146865 = 0;
      *progress = 0;
      func_8019FA28(0x28u, 0xac0000u, 0x210000u, 0x83u);
      D_80146875 = 0x18u;
      break;
    }

    case 24:
      if (g_ScenarioProgress != 9u) {
        break;
      }

      func_8015D4F8(0u, 3u, 0x64, 0x10);
      func_801636A0(3, 1);
      D_80146875 = 0x19u;
      break;

    case 25: {
      u8* progress;

      if ((u8)func_80163EA0() == 0) {
        break;
      }

      progress = &g_ScenarioProgress;
      D_80146875 = 0x1bu;
      *progress = *progress + 1;
      break;
    }

    case 26: {
      u8* progress;

      if (func_80162D00() == 0) {
        break;
      }

      progress = &g_ScenarioProgress;
      D_80146875 = 0x1bu;
      *progress = *progress + 1;
      break;
    }

    case 28:
      if (D_80143BB0 == 2) {
        break;
      }

      D_8014865C = -2;
      D_80148698 = 1u;
      D_80143B90 = 7u;
      D_80148650 = 0;
      D_80148699 = 0;
      D_80146875 = 0x1du;
      break;

    case 29:
      if (D_80143B90 != 2u) {
        break;
      }

      /* Substate 29 shares the closing arm of substate 30. */
    case 30:
      func_80161C20(D_80145029, 0x64, 8);
      g_ScenarioProgress = 0;
      func_8015C058();
      D_80146874 = 0;
      D_80146875 = 0;
      break;
  }
}
