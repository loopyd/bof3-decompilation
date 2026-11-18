#include "bof3/bof3.h"

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x8014686C @kind data */
extern u32 D_8014686C;
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x8014496F @kind data */
extern u8 D_8014496F[];
/* @source 0x80181B12 @kind unknown */
extern u8 D_80181B12;
/* @source 0x80181B16 @kind unknown */
extern u8 D_80181B16;
/* @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;
/* Shared 0x74-byte front-end record table; a byte pointer keeps its element
 * stride, so every store below carries the 0x74 scale itself. */
/* @source 0x80143FC8 @kind unknown */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FD1[];
/* @source 0x80143FD4 @kind unknown */
extern u8 D_80143FD4[];
/* @source 0x8014402C @kind unknown */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind unknown */
extern u8 D_80144030[];
/* @source 0x80144034 @kind unknown */
extern u8 D_80144034[];
/* @source 0x801492D8 @kind unknown */
extern s16 D_801492D8;
/* @source 0x801492DA @kind unknown */
extern s16 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern s16 D_801492DC;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015D404(u32 arg0, s32 arg1);
void func_8015DF18(u16 arg0);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_80166180(s32 arg0);
void func_8019FA28(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
void func_801C16B8(void);
void func_801C601C(u32 arg0);
u8 func_8019601C(void);

/* @source 0x801F8188
 * @behavior Word 3 (0-based) of the eighteen-word primary state table at
 *           0x801FE3E4, which dispatchPrimaryStateHandlerScenarioScena0600_801F7A00
 *           indexes with the shared primary scene state byte D_80146874. It
 *           steps the shared secondary scene state byte D_80146875 through a
 *           sixteen-entry case table in which only states 2 and 6 have no body;
 *           those two, and every value above 0x0F, fall to the epilogue.
 *           State 0 arms secondary state 1, runs func_8019FA28 with 0x2F,
 *           0x718000, 0x90000 and 0x80 and then stores 0xFF in D_80143F1F.
 *           State 1, while the shared halfword D_80143C40 reads zero, raises the
 *           progress byte to 1, runs func_801C601C with mode 0 and advances to
 *           state 3. States 3, 5, 9, 11 and 12 all store the object byte
 *           func_8019601C in the scratchpad byte 0x1F800000 and, unless that
 *           byte is 0xFF, seed the selected 0x74-byte front-end record at
 *           D_80143FC8 with flag 1 and byte 0x13 at +5, word +0x64, word +0x68,
 *           word +0x6C and byte +9, differing only in those four literals and
 *           in the state they arm: state 3, at progress 3, writes 0x31, the
 *           signed halfwords D_801492D8 and D_801492DA, 0x124, 0x700 and 0x5A
 *           and advances to state 4; state 5, at progress 0x0A, writes 0x13,
 *           -0x3B5, D_801492DA, D_801492DC and 0x1E and advances to state 7;
 *           state 9, at progress 0x0D, writes the same words as state 5 through
 *           -0x2AA and advances to state 0x0A; state 11 has no progress guard,
 *           runs func_801C601C with mode 0 first, writes zero in word +0x64 with
 *           D_801492DA and D_801492DC in the next two and 2 at +9, and advances
 *           to state 0x0C; state 12, at progress 1, writes -0x2AA, D_801492DA,
 *           D_801492DC and 0x78 and advances to state 0x0D.
 *           State 4, at progress 7, runs func_80161C20 with 0x27, 0x64 and 8,
 *           sets flag bit 8 of D_80146258 and advances to state 5. State 7 runs
 *           func_8015D404 with 0 and 2 and func_8015DF18 with 0x202 before
 *           arming state 8. State 8, at progress 0x0C, runs func_80161C20 with
 *           0x24, 0x64 and 8 and func_8015DF18 with 0x203, toggles bit 8 of
 *           D_80146258 and advances to state 9. State 10, at progress 0x0E,
 *           arms state 0x0B, sets that same flag bit, hands the scenario flag
 *           word D_8014686C and mode 0x0A to func_8015B580 and runs
 *           func_8019FA28 with 0x35, 0x230000, 0x290000 and 0x80. State 13, at
 *           progress 2, arms state 0x0E, hands D_8014686C to func_8015B580 twice
 *           with modes 0x0B and 9 and runs func_80166180 with 4. State 14, at
 *           progress 0x32, arms state 0x0F, toggles bit 8 of D_80146258, raises
 *           bit 0 of the 0xA4-byte field record that D_80181B16 selects at
 *           D_8014496F and of the one D_80181B12 selects, and runs func_8019FA28
 *           with 0x2D, 0x220000, 0x2A0000 and 3. State 15 runs func_801C16B8
 *           and the shared release helper func_8015C058, then clears
 *           g_ScenarioProgress, D_80146865 to D_80146867, the secondary and the
 *           primary state bytes. The 0x18-byte frame keeps $ra alone, and the
 *           sixteen-word case table sits in the overlay rodata slot at
 *           0x801F6CBC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState3ScenarioScena0600_801F8188(
    void) {
  switch (D_80146875) {
    case 0:
      D_80146875 = 1u;
      func_8019FA28(0x2Fu, 0x718000u, 0x90000u, 0x80u);
      D_80143F1F = 0xFFu;
      return;

    case 1:
      if (D_80143C40 != 0u) {
        return;
      }

      g_ScenarioProgress = 1u;
      func_801C601C(0u);
      D_80146875 = 3u;
      return;

    case 3: {
      u8* scratch;
      u8 object_index;

      if (g_ScenarioProgress != 3u) {
        return;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x31u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = (s32)D_801492D8;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x124;
        *(s32*)(D_80143FD4 + (u32)*scratch * 0x74u) = 0x700;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x5Au;
        D_80146875 = 4u;
      }
      return;
    }

    case 4:
      if (g_ScenarioProgress != 7u) {
        return;
      }

      func_80161C20(0x27u, 0x64, 8);
      D_80146875 = 5u;
      D_80146258 |= 8u;
      return;

    case 5: {
      u8* scratch;
      u8 object_index;

      if (g_ScenarioProgress != 0x0Au) {
        return;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x3B5;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x1Eu;
        D_80146875 = 7u;
      }
      return;
    }

    case 7:
      func_8015D404(0u, 2);
      func_8015DF18(0x202u);
      D_80146875 = 8u;
      return;

    case 8:
      if (g_ScenarioProgress != 0x0Cu) {
        return;
      }

      func_80161C20(0x24u, 0x64, 8);
      func_8015DF18(0x203u);
      D_80146875 = 9u;
      D_80146258 ^= 8u;
      return;

    case 9: {
      u8* scratch;
      u8 object_index;

      if (g_ScenarioProgress != 0x0Du) {
        return;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x1Eu;
        D_80146875 = 0x0Au;
      }
      return;
    }

    case 10:
      if (g_ScenarioProgress != 0x0Eu) {
        return;
      }

      D_80146875 = 0x0Bu;
      D_80146258 |= 8u;
      func_8015B580(D_8014686C, 0x0A);
      func_8019FA28(0x35u, 0x230000u, 0x290000u, 0x80u);
      return;

    case 11: {
      u8* scratch;
      u8 object_index;

      func_801C601C(0u);
      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = 0;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 2u;
        D_80146875 = 0x0Cu;
      }
      return;
    }

    case 12: {
      u8* scratch;
      u8 object_index;

      if (g_ScenarioProgress != 1u) {
        return;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xFFu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x78u;
        D_80146875 = 0x0Du;
      }
      return;
    }

    case 13:
      if (g_ScenarioProgress != 2u) {
        return;
      }

      D_80146875 = 0x0Eu;
      func_8015B580(D_8014686C, 0x0B);
      func_8015B580(D_8014686C, 9);
      func_80166180(4);
      return;

    case 14: {
      s32 setOffset;
      s32 secondOffset;

      if (g_ScenarioProgress != 0x32u) {
        return;
      }

      D_80146875 = 0x0Fu;
      D_80146258 ^= 8u;
      setOffset = (u32)D_80181B16 * 0xA4u;
      D_8014496F[setOffset] = D_8014496F[setOffset] | 1;
      secondOffset = (u32)D_80181B12 * 0xA4u;
      D_8014496F[secondOffset] = D_8014496F[secondOffset] | 1;
      func_8019FA28(0x2Du, 0x220000u, 0x2A0000u, 3u);
      return;
    }

    case 15:
      func_801C16B8();
      func_8015C058();
      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;
  }
}
