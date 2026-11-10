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
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x801492DA @kind unknown */
extern s16 D_801492DA;
/* @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;
/* @source 0x80143FC8 @kind unknown */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FD1[];
/* @source 0x8014402C @kind unknown */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind unknown */
extern u8 D_80144030[];
/* @source 0x80144034 @kind unknown */
extern u8 D_80144034[];

void func_8014ECAC(u16 local_mode);
void func_8015C058(void);
void func_8015B580(u32 arg0, s32 arg1);
s32 func_8019601C(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801FC718
 * @behavior Primary state 17 of this overlay, reached as word 17 of the
 *           overlay handler table at 0x801FE3E4: it steps the shared secondary
 *           scene state byte D_80146875 through eleven states, each gated on
 *           the shared front-end progress byte g_ScenarioProgress.
 *           State 0 arms secondary state 1 and requests the front selector
 *           context (0x2E, 0xF8000, 0x2B0000, 0x80); state 1 requests
 *           front-end mode 1, arms effect bank 0x1F in D_8014832E and advances
 *           to state 2; state 2, while the gate word D_80143C40 is clear,
 *           raises the progress byte to 3 and advances to state 3; state 3,
 *           at progress 0x0F, advances to state 4 and sets bit 3 of the mode
 *           halfword D_80146258; states 4 and 5, at progress 0x14 and 0x18,
 *           store the object byte func_8019601C in scratchpad 0x1F800000 and,
 *           unless that byte is 0xFF, seed the selected 0x74-byte front-end
 *           record at 0x80143FC8 (+0 = 1, +5 = 0x13, +9 = 0x1E, the signed
 *           word +0x64 = -0x2AA, +0x68 = the signed halfword D_801492DA and
 *           +0x6C = 0xF8 in state 4 or 0x200 in state 5) before advancing the
 *           secondary state; state 6, at progress 0x1B, advances to state 7 and
 *           requests the front selector context (0x3C, 0x2F0000, 0x530000,
 *           0x80); state 7 advances to state 8 and inverts bit 3 of D_80146258;
 *           state 8, at progress 9, advances to state 0x0A, sets flag bit 0x3D
 *           in the scenario flag word D_8014686C through func_8015B580 and
 *           requests the front selector context (0x57, 0x220000, 0x390000, 1);
 *           state 9 is empty; state 10 runs the shared overlay release helper
 *           func_8015C058 and clears g_ScenarioProgress and the shared bytes
 *           0x80146865..0x80146867, the secondary state byte and the primary
 *           state byte D_80146874. Every other secondary state value falls to
 *           the epilogue. The 0x18-byte frame only keeps $ra across the calls
 *           and the state-byte load is hoisted above the frame setup; the
 *           eleven-entry case table sits in the overlay rodata slot at
 *           0x801F724C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState17ScenarioScena0600_801FC718(
    void) {
  switch (D_80146875) {
    case 0:
      D_80146875 = 1u;
      func_8019FA28(0x2Eu, 0xF8000u, 0x2B0000u, 0x80u);
      break;

    case 1:
      func_8014ECAC(1u);
      D_8014832E = 0x1Fu;
      D_80146875 = 2u;
      break;

    case 2:
      if (D_80143C40 != 0u) {
        break;
      }

      g_ScenarioProgress = 3u;
      D_80146875 = 3u;
      break;

    case 3:
      if (g_ScenarioProgress == 0xFu) {
        D_80146875 = 4u;
        D_80146258 |= 8u;
      }
      break;

    case 4:
      if (g_ScenarioProgress == 0x14u) {
        u8* scratch;
        u8 object_index;

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xFFu) {
          D_80143FC8[(u32)object_index * 0x74u] = 1u;
          D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
          *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
          *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
          *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0xF8;
          D_80143FD1[(u32)*scratch * 0x74u] = 0x1Eu;
          D_80146875 = 5u;
        }
      }
      break;

    case 5:
      if (g_ScenarioProgress == 0x18u) {
        u8* scratch;
        u8 object_index;

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xFFu) {
          D_80143FC8[(u32)object_index * 0x74u] = 1u;
          D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
          *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
          *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
          *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
          D_80143FD1[(u32)*scratch * 0x74u] = 0x1Eu;
          D_80146875 = 6u;
        }
      }
      break;

    case 6:
      if (g_ScenarioProgress == 0x1Bu) {
        D_80146875 = 7u;
        func_8019FA28(0x3Cu, 0x2F0000u, 0x530000u, 0x80u);
      }
      break;

    case 7:
      D_80146875 = 8u;
      D_80146258 ^= 8u;
      break;

    case 8:
      if (g_ScenarioProgress == 9u) {
        D_80146875 = 0xAu;
        func_8015B580(D_8014686C, 0x3D);
        func_8019FA28(0x57u, 0x220000u, 0x390000u, 1u);
      }
      break;

    case 9:
      break;

    case 10:
      func_8015C058();
      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
  }
}
