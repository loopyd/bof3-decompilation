#include "bof3/bof3.h"

/* Shared scenario progress byte: the progress-gated arms compare against 0, 1,
 * 3, 0xB, 0x12, 0x14, 0x15, 0x16 and 0x23, one arm seeds it with 0xF, five arms
 * raise it by one and two reset arms clear it.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte: three arms return while it still reads 2 and the
 * four queue arms publish 2 in it.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary state byte of the front-end state tuple, cleared by the two
 * reset tails.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the same tuple and the switch operand of
 * this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared tick halfword: four arms seed it with 0x3C, 0x32, 0x28, 0x1E or 0x5A,
 * two arms decrement it once per frame and one arm decrements it and then hands
 * the frame to the move helper.
 * @source 0x80146876 @kind data
 */
extern u16 D_80146876;

/* Shared scenario byte the first arm compares against 0x14 before it stages the
 * opening effect.
 * @source 0x80146867 @kind data
 */
extern u8 D_80146867;

/* Shared flag-bank word the seeding arms pass to the bit helper.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

/* Shared gate halfword: the palette, tick and queue arms hold until it reads
 * zero while the party-scan and record arms hold while it reads nonzero.
 * @source 0x80143C40 @kind data
 */
extern u16 D_80143C40;

/* Shared flash byte the palette arms raise.
 * @source 0x8014832E @kind data
 */
extern u8 D_8014832E;

/* Shared selection index the two front-end cue arms scale by four into the cue
 * table pair D_80181EBA and D_80181EBB.
 * @source 0x80145029 @kind data
 */
extern u8 D_80145029;

/* Shared live count of the 0x140-byte-stride local work records.
 * @source 0x80146254 @kind data
 */
extern u8 D_80146254;

/* Party byte (+0x79) of the 0x140-byte-stride local work records based at
 * 0x80145E90.
 * @source 0x80145F09 @kind unknown
 */
extern u8 D_80145F09[];

/* Shared position halfword the move-helper arms pass as the second coordinate.
 * @source 0x801492DA @kind data
 */
extern s16 D_801492DA;

/* Shared 0x74-byte field-record table: the object byte func_8019601C answers
 * selects the row whose occupancy byte, byte 1 and byte 5 the seeding arms
 * write.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];
/* @source 0x80143FC9 @kind table
 */
extern u8 D_80143FC9[];
/* @source 0x80143FCD @kind table
 */
extern u8 D_80143FCD[];

/* Shared row colour bytes the effect arms set to 0xFF, 0xE or 8.
 * @source 0x80143F1D @kind data
 */
extern u8 D_80143F1D;
/* @source 0x80143F1E @kind data
 */
extern u8 D_80143F1E;
/* @source 0x80143F1F @kind data
 */
extern u8 D_80143F1F;

/* Shared flag bytes the effect arms raise bit 6 of.
 * @source 0x80145FD0 @kind data
 */
extern u8 D_80145FD0;
/* @source 0x80146110 @kind data
 */
extern u8 D_80146110;
/* @source 0x80145E90 @kind data
 */
extern u8 D_80145E90;

/* Shared 0x2E8-byte effect row the first arm hands to the flag helper.
 * @source 0x80144F28 @kind unknown
 */
extern u8 D_80144F28[];

/* Shared object-index cell: the seeding arms read the byte helper's answer back
 * from it for every row cell after the first.
 * @source 0x801FC4C0 @kind data
 */
extern u8 D_801FC4C0;

/* Shared candidate byte rows and paired halfword action rows this overlay keeps
 * in its own trailing table data.
 * @source 0x801FC450 @kind table
 */
extern u8 D_801FC450[];
/* @source 0x801FC454 @kind table
 */
extern u8 D_801FC454[];
/* @source 0x801FC458 @kind table
 */
extern s16 D_801FC458[];
/* @source 0x801FC460 @kind table
 */
extern s16 D_801FC460[];

/* Shared 4-byte cue rows the front-end cue arms index with D_80145029: byte 0 is
 * the first seed and byte 1 the second.
 * @source 0x80181EBA @kind table
 */
extern u8 D_80181EBA[];
/* @source 0x80181EBB @kind table
 */
extern u8 D_80181EBB[];

/* Scratchpad gate byte of the shared main executable that the record-scan arm
 * walks as its own loop index.
 * @source 0x1F800000 @kind data
 */
extern u8 D_1F800000[];

u16 audioActionPresentCandidate(
    u8* candidates, s16* actions);
u8 func_801F7A18(u32 arg0, s16 arg1, s16 arg2, s32 arg3, u8 arg4, u32 arg5);
u8 func_8019601C(void);
void func_8015C088(void);
void func_801BE1B0(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_8014ECAC(u16 arg0);
void func_80150224(s32 action_id);
void func_8015C058(void);
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void func_8015D404(u32 arg0, s32 arg1);
void func_80161BBC(u32 selection_id);
void func_8015DF18(u32 arg0);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void averagePaletteChannels(void);

/* @source 0x801F9834
 * @behavior Primary-state-6 handler of this scenario overlay and sub-state
 *           dispatcher of its front-end entry chain: the payload keeps its
 *           address once, as word 6 of the ten-word handler table at 0x801FC424
 *           that dispatchPrimaryStateHandlerScenarioScena1400_801F7B5C indexes
 *           with the shared primary state byte D_80146874, and no in-image jal
 *           names it. It steps the shared secondary sub-state byte D_80146875
 *           through the 74-slot jump table the payload keeps at 0x801F6E2C,
 *           reading the byte with lbu, bounding it with sltiu against 0x4A and
 *           hoisting both above the frame setup; slots 8 and 9 and every value
 *           outside the table return untouched. Sub-state 0 releases the shared
 *           front end through func_8015C088 and queues cue 0, sub-state 1 stages
 *           the opening effect through func_8015B580 and func_8019FA28 once the
 *           shared scenario byte reads 0x14, sub-states 2 and 5 run the overlay
 *           party-scan action starter over their own candidate rows while the
 *           shared object row bit 0 is clear, and sub-states 3, 6 and 0x14 hold
 *           on the front-end phase byte. Sub-state 0x0B walks the
 *           0x140-byte-stride work records through the scratchpad index byte
 *           until one carries party byte 4. Sub-states 0x0C, 0x0D and 0x40 to
 *           0x42 claim the object row the byte helper func_8019601C answers from
 *           the shared object-index cell and raise the row colour bytes, while
 *           0x0E to 0x11, 0x15 and 0x43 to 0x45 stage the effect rows of the
 *           three effect helpers and 0x12, 0x13 and 0x46 to 0x48 seed the
 *           front-end cue pair of the shared selection index. Sub-states 0x16 to
 *           0x1A cycle the palette-average and flash-step heads, the two tick
 *           arms seed the countdown, and each of those arms advances one
 *           sub-state. Sub-states 0x17 to 0x3F repeat that cycle with six
 *           effect bands, and 0x49 clears the state tuple after the release
 *           step and cue 8. The body takes no arguments, returns nothing and
 *           uses a 0x20-byte frame that only keeps $ra at 0x18, because the two
 *           outgoing argument words of the move helper sit at 0x10 and 0x14.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation: case 0x0B retains the byte load in a word-sized count for
 * the entry guard, then captures its byte-sized loop bound inside that guard.
 * Its loop-local scratchpad pointer lets loop-invariant motion create the
 * preheader-to-loop copy. With a byte count alone that pointer shape retained
 * an extra entry mask; with a word count alone it forwarded the increment
 * instead of reloading the scratchpad byte. The split count phases preserve
 * the unmasked guard and the loop reload at the original 0x10D0-byte size.
 */
void dispatchSubStateByteTable6E2CScenarioScena1400_801F9834(void) {
  switch (D_80146875) {
  case 0:
    func_8015C088();
    func_801BE1B0(0);
    D_80146875 = 1;
    return;
  case 1:
    if (D_80146867 != 0x14) {
      return;
    }
    func_8015B580((u32)D_80144F28, 0x4E);
    func_8019FA28(0xAC, 0x260000, 0x360000, 0x84);
    D_80146875 = 2;
    return;
  case 2:
    if (D_80143C40 != 0) {
      return;
    }
    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1u) != 0) {
      return;
    }
    audioActionPresentCandidate(
        D_801FC450, D_801FC458);
    D_80146875 = 3;
    return;
  case 3:
    if (D_80143BB0 == 2) {
      return;
    }
    g_ScenarioProgress = 0xA;
    D_80146875 = 4;
    return;
  case 4:
    if (g_ScenarioProgress != 0xB) {
      return;
    }
    if (func_801F7A18(0x31, -0x2AA, D_801492DA, 0x200, 0x1E, 0) == 0) {
      return;
    }
    D_80146875 = 5;
    return;
  case 5: {
    u8* progress;

    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1u) != 0) {
      return;
    }
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    audioActionPresentCandidate(
        D_801FC454, D_801FC460);
    D_80146875 = 6;
    return;
  }
  case 6: {
    u8* progress;

    if (D_80143BB0 == 2) {
      return;
    }
    D_80146875 = 7;
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    return;
  }
  case 7:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_8015B580(D_8014686C, 0x12);
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  case 0x0A:
    g_ScenarioProgress = 0xF;
    func_801BE1B0(5);
    D_80146875 = 0x0B;
    return;
  case 0x0B: {
    u32 count;
    u32 offset;

    if (g_ScenarioProgress != 0x12) {
      return;
    }
    D_80146875 = 0x0C;
    count = D_80146254;
    D_1F800000[0] = 0;
    if (count != 0) {
      u8 loop_count = count;
      do {
        u8* index = D_1F800000;
        offset = (u32)*index * 0x140;
        if (D_80145F09[offset] == 4) {
          func_801BE1B0(6);
          return;
        }
        *index = *index + 1;
      } while (D_1F800000[0] < loop_count);
    }
    func_801BE1B0(7);
    return;
  }
  case 0x0C: {
    u8 object_index;

    if (g_ScenarioProgress != 0x23) {
      return;
    }
    object_index = func_8019601C();
    D_801FC4C0 = object_index;
    if (object_index != 0xFF) {
      D_80143FC8[(u32)object_index * 0x74u] = 1;
      D_80143FCD[(u32)D_801FC4C0 * 0x74u] = 0x94;
    }
    D_80146876 = 0x5A;
    D_80146875 = 0x0D;
    return;
  }
  case 0x0D: {
    u16* count;
    u8* progress;

    count = &D_80146876;
    if (--*count != 0) {
      return;
    }
    D_80146875 = 0x0E;
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    return;
  }
  case 0x0E:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_8015B580(D_8014686C, 0x13);
    func_8019FA28(0xB, 0x1E0000, 0xB0000, 0);
    D_80143F1D = 0xFF;
    D_80146875 = 0x0F;
    return;
  case 0x0F:
    if (g_ScenarioProgress != 1) {
      return;
    }
    D_8014832E = 0x1F;
    func_8014ECAC(1);
    D_80146875 = 0x10;
    return;
  case 0x10: {
    u8* progress;

    if (D_80143C40 == 0) {
      D_80146875 = 0x11;
      progress = &g_ScenarioProgress;
      *progress = *progress + 1;
    }
  }
  case 0x11:
    if (g_ScenarioProgress != 3) {
      return;
    }
    func_801F7A18(0x13, -0x2AA, D_801492DA, 0x200, 0x80, 0);
    D_80146875 = 0x12;
    return;
  case 0x12:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_8014ECAC(0);
    func_8015D4F8(D_80181EBA[(u32)D_80145029 * 4],
                  D_80181EBB[(u32)D_80145029 * 4], 0x64, 0x20);
    D_80146875 = 0x13;
    return;
  case 0x13:
    if (D_80143C40 != 0) {
      return;
    }
    func_8015D404(D_80181EBA[(u32)D_80145029 * 4],
                  D_80181EBB[(u32)D_80145029 * 4]);
    D_8014832E = 0;
    func_80150224(0x30);
    D_80143BB0 = 2;
    D_80146875 = 0x14;
    return;
  case 0x14:
    if (D_80143BB0 != 2) {
      D_80146875 = 0x15;
    }
    return;
  case 0x15:
    func_8019FA28(0x33, 0x30000, 0x70000, 0x82);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x16;
    return;
  case 0x16:
  case 0x1C:
  case 0x22:
  case 0x28:
  case 0x2E:
  case 0x34:
  case 0x3A: {
    u8* state;

    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0x1F;
    averagePaletteChannels();
    func_8014ECAC(1);
    state = &D_80146875;
    *state = *state + 1;
    return;
  }
  case 0x17:
  case 0x1D:
  case 0x23:
  case 0x29:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146876 = 0x3C;
    D_80146875 = D_80146875 + 1;
    return;
  case 0x18:
  case 0x1E:
  case 0x24:
  case 0x2A:
  case 0x30:
  case 0x36:
  case 0x3C: {
    u16* count;

    count = &D_80146876;
    if (--*count != 0) {
      return;
    }
    func_8014ECAC(0);
    D_80146875 = D_80146875 + 1;
    return;
  }
  case 0x19:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x27);
    D_80143BB0 = 2;
    D_80146875 = 0x1A;
    return;
  case 0x1A:
  case 0x20:
  case 0x26:
  case 0x2C:
  case 0x32:
  case 0x38:
  case 0x3E: {
    u8* state;

    if (D_80143BB0 == 2) {
      return;
    }
    state = &D_80146875;
    *state = *state + 1;
    return;
  }
  case 0x1B:
    func_8019FA28(0x43, 0x1E0000, 0x360000, 0x86);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x1C;
    return;
  case 0x1F:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x41);
    D_80143BB0 = 2;
    D_80146875 = 0x20;
    return;
  case 0x21:
    func_8019FA28(0x55, 0x268000, 0x438000, 0x83);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x22;
    return;
  case 0x25:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x14);
    D_80143BB0 = 2;
    D_80146875 = 0x26;
    return;
  case 0x27:
    func_8019FA28(0x2B, 0x110000, 0x190000, 0);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x28;
    return;
  case 0x2B:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x27);
    D_80143BB0 = 2;
    D_80146875 = 0x2C;
    return;
  case 0x2D:
    func_8019FA28(0x67, 0x4C0000, 0x1F0000, 0);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x2E;
    return;
  case 0x2F:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146876 = 0x32;
    D_80146875 = 0x30;
    return;
  case 0x31:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0xA);
    D_80143BB0 = 2;
    D_80146875 = 0x32;
    return;
  case 0x33:
    func_8019FA28(0x78, 0x1F8000, 0x748000, 0);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x34;
    return;
  case 0x35:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146876 = 0x28;
    D_80146875 = 0x36;
    return;
  case 0x37:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x49);
    D_80143BB0 = 2;
    D_80146875 = 0x38;
    return;
  case 0x39:
    func_8019FA28(0x90, 0x230000, 0x1D8000, 0);
    D_80143F1E = 0xFF;
    D_80143F1D = 0xFF;
    D_80143F1F = 0xFF;
    D_80146875 = 0x3A;
    return;
  case 0x3B:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146876 = 0x1E;
    D_80146875 = 0x3C;
    return;
  case 0x3D:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    func_80150224(0x49);
    D_80143BB0 = 2;
    D_80146875 = 0x3E;
    return;
  case 0x3F:
    func_8019FA28(0xAC, 0x68000, 0x2C8000, 0);
    D_80143F1E = 0xFF;
    D_80143F1F = 8;
    D_80146875 = 0x40;
    return;
  case 0x40: {
    u8 object_index;
    u8* flag;

    if (g_ScenarioProgress != 0x14) {
      return;
    }
    object_index = func_8019601C();
    D_801FC4C0 = object_index;
    if (object_index != 0xFF) {
      D_80143FC8[(u32)object_index * 0x74u] = 1;
      D_80143FCD[(u32)D_801FC4C0 * 0x74u] = 0x94;
      D_80143FC9[(u32)D_801FC4C0 * 0x74u] = 3;
    }
    flag = &D_80145FD0;
    *flag = *flag | 0x40;
    D_80146110 = D_80146110 | 0x40;
    func_8015DF18(0x208);
    D_80146875 = 0x41;
    return;
  }
  case 0x41: {
    u8 object_index;

    if (g_ScenarioProgress != 0x15) {
      return;
    }
    object_index = func_8019601C();
    D_801FC4C0 = object_index;
    if (object_index != 0xFF) {
      D_80143FC8[(u32)object_index * 0x74u] = 1;
      D_80143FCD[(u32)D_801FC4C0 * 0x74u] = 0x94;
      D_80143FC9[(u32)D_801FC4C0 * 0x74u] = 3;
    }
    func_8015DF18(0x208);
    D_80146875 = 0x42;
    return;
  }
  case 0x42: {
    u8 object_index;
    u8* flag;

    if (g_ScenarioProgress != 0x16) {
      return;
    }
    object_index = func_8019601C();
    D_801FC4C0 = object_index;
    if (object_index != 0xFF) {
      D_80143FC8[(u32)object_index * 0x74u] = 1;
      D_80143FCD[(u32)D_801FC4C0 * 0x74u] = 0x94;
      D_80143FC9[(u32)D_801FC4C0 * 0x74u] = 3;
    }
    flag = &D_80145E90;
    *flag = *flag | 0x40;
    func_8015DF18(0x208);
    D_80146875 = 0x43;
    return;
  }
  case 0x43:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_8019FA28(0x8D, 0x5F8000, 0xA8000, 0);
    D_80143F1D = 0xE;
    D_80143F1F = 0xFF;
    D_80146875 = 0x44;
    return;
  case 0x44: {
    u8* progress;

    if (D_80143C40 != 0) {
      return;
    }
    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1u) != 0) {
      return;
    }
    func_80161BBC(0x50);
    D_80146875 = 0x45;
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    return;
  }
  case 0x45:
    if (g_ScenarioProgress != 3) {
      return;
    }
    if (func_801F7A18(0x13, -0x344, D_801492DA, 0x34A, 0x20, 0) == 0) {
      return;
    }
    D_80146875 = 0x46;
    return;
  case 0x46:
    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1u) != 0) {
      return;
    }
    func_8015B580(D_8014686C, 0x14);
    D_80146876 = 0x1E;
    D_80146875 = 0x47;
    return;
  case 0x47: {
    u16* count;
    u8* progress;

    count = &D_80146876;
    if (--*count != 0) {
      return;
    }
    if (func_801F7A18(0x13, -0x2AA, D_801492DA, 0x200, 0x20, 0) == 0) {
      return;
    }
    D_80146875 = 0x48;
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    return;
  }
  case 0x48: {
    u8* progress;

    if ((D_80143FC8[(u32)D_801FC4C0 * 0x74u] & 1u) != 0) {
      return;
    }
    D_80146875 = 0x49;
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    return;
  }
  case 0x49:
    if (g_ScenarioProgress != 0) {
      return;
    }
    func_8015C058();
    func_80161C20(0x50, 0x64, 8);
    D_80146874 = 0;
    D_80146875 = 0;
    return;
  }
}
