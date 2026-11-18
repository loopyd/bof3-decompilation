#include "bof3/bof3.h"

/* Shared scenario progress byte at 0x80146864: almost every arm gates on it,
 * four arms clear it with one address taken into a local pointer, and
 * three arms publish 1 or 2 into it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte at 0x80143BB0: the arms that leave a window
 * store 2 into it, the two arms that open one require it to read 2, and the
 * 0x94 arm advances it to 6.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared gate halfword at 0x80143C40, read with lhu: four arms wait for it to
 * read zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared mode halfword at 0x80146258, read with lhu: two arms set bit 3 and
 * bit 7 of it, and the two closing tails toggle bit 3.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared effect bank byte at 0x8014832E: the arms that arm a window or a
 * front-end request publish 0x1F or 0 into it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared cue byte at 0x80143F1F: the 0x5B arm stores 0x87 here.
 * @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;

/* Shared scenario flag word at 0x8014686C, read with lw: the arms that set a
 * scenario flag bit pass it to the bit-set helper.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared primary state byte at 0x80146874: the closing tails clear it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte at 0x80146875: this sequence switches on it, and every
 * arm that advances the sequence stores it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared route byte at 0x80146865: the 0x96 arm clears it with the two bytes
 * below.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared party byte at 0x80146866: the 0x5E, 0x72, 0x6F and 0x78 arms publish
 * 1, 2 or 0 into it and the 0x96 arm clears it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared step counter byte at 0x80146867: the four 0x4F-0x52 arms step it and
 * reset it, and the 0x96 arm clears it.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared front-end work byte at 0x801460FB, read with lbu: the 0x46, 0x6E and
 * 0x78 arms leave while it reads 3.
 * @source 0x801460FB @kind unknown */
extern u8 D_801460FB;

/* Shared fixed-point halfword at 0x8014932C, read with lhu and stored with sh:
 * the four 0x4F-0x52 arms step it by 0x20, -0x10, 0x80 or -0x80 and then reset
 * it to 0x280, 0, 0x500 or 0.
 * @source 0x8014932C @kind data */
extern u16 D_8014932C;

/* Shared area mode byte at 0x80149333: every 0x4F-0x53 arm publishes 2 into it,
 * the four stepping arms twice on their closing path.
 * @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Shared byte flag bank at 0x80144EB8: the 0x14 and 0x5E arms set bit 0x3E of
 * it, and the 0x46 arm reaches it as the word 0x70 below D_80144F28.
 * @source 0x80144EB8 @kind data */
extern u8 D_80144EB8;

/* Second shared byte flag bank at 0x80144F28, 0x70 bytes above D_80144EB8: the
 * 0x46 arm sets bit 0x45 of it, the 0x7E arm bit 0x40 and the closing 0x96 arm
 * bit 0x33.
 * @source 0x80144F28 @kind data */
extern u8 D_80144F28;

/* Shared window opener at 0x801BE1B0: the arms that open a window run it with
 * the selector the original encodes as 1, 2, 9 or 0xA.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared overlay reset helper at 0x801C601C: the arms that step from one window
 * to the next run it with the window selector 1, 3, 6 or 7.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared front-end mode setter at 0x8014ECAC: six arms set mode 1, 4, 5 or 0xD.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared scenario flag-word bit-set helper at 0x8015B580: the arms pass it the
 * shared flag word, one of the two byte flag banks, and a bit index.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared scenario flag-word bit-clear helper at 0x8015B5A8.
 * @source 0x8015B5A8 @kind function */
extern void func_8015B5A8(u32 arg0, s32 arg1);

/* Shared overlay release helper at 0x8015C058: every closing tail runs it
 * before it clears the state bytes.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end request setter at 0x8019FA28: the arms pass it a selection
 * seed, two 16.16 context words and the request kind the original encodes.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared front-end cue queue at 0x80150224: eleven arms queue one cue id.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared front-end handler runners at 0x801C1E3C and 0x801C1DF0: the 0x0C and
 * 0x8C arms run one handler index.
 * @source 0x801C1E3C @kind function */
extern void func_801C1E3C(u8 index);
/* @source 0x801C1DF0 @kind function */
extern void func_801C1DF0(u8 index);

/* Shared field-record reinitializer at 0x801C16B8, run by the 0x90 arm.
 * @source 0x801C16B8 @kind function */
extern void func_801C16B8(void);

/* Shared two-argument controller-step helper at 0x801636A0: the 0x4D arm runs
 * it with (2, 1) and the 0x91 arm with (0, 1).
 * @source 0x801636A0 @kind function */
extern void func_801636A0(u32 arg0, u32 arg1);

/* Shared record-state query at 0x80163EA0, returning the live record byte
 * zero-extended into a word: the 0x4E and 0x92 arms advance only while its low
 * byte is nonzero.
 * @source 0x80163EA0 @kind function */
extern u32 func_80163EA0(void);

/* Shared front-end cue finisher at 0x80161D74, run by the 0x4E arm with 0.
 * @source 0x80161D74 @kind function */
extern void func_80161D74(s32 arg0);

/* Shared four-argument scripted-motion starter at 0x8015D554: the 0x90 arm
 * starts it with (0, 0, 4, 0x1E).
 * @source 0x8015D554 @kind function */
extern void func_8015D554(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @source 0x801FAE0C
 * @behavior Word 15 (the word stored at 0x801FE428) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801FAE0C and that table word is its
 * only reference in this payload. It reads the shared sub-state byte D_80146875
 * before it creates its 0x18-byte frame, which keeps only $ra and $s0, and
 * switches on it through the 151-entry absolute jump table
 * jtbl_801F706C_main at 0x801F706C, an overlay-local table in this payload's
 * leading tabledata blob that names a body for sixty-six of its entries and
 * returns for the other eighty-five. The bodies are a window sequence coded in
 * decimal decades: substates 0 and 1 open, 10 to 22 set flags and step the
 * shared scenario progress g_ScenarioProgress (0x80146864), 30 to 52 run the
 * 0x46-class cue banks, 60 to 62 arm mode bits, 70 to 83 step the shared
 * fixed-point halfword D_8014932C (0x8014932C, read with lhu and stored with
 * sh) against the step counter byte D_80146867 (0x80146867), 90 to 94 seed a
 * front selector request, 100 to 101 close a cue bank, 110 to 114 repeat the
 * mode-bit pattern, 120 to 127 seed three more front selector requests, 130 and
 * 131 close one, and 140 to 150 run the final window steps. The eleven arms
 * that close the state (1, 12, 16, 22, 41, 51, 62, 101, 127, 131 and 150) each
 * run func_8015C058 and then clear the sub-state and primary state bytes in
 * place; the 150 arm clears g_ScenarioProgress together with the three shared
 * bytes D_80146865, D_80146866 and D_80146867 before those two, and its body
 * falls into the closing block the other ten reach by jump, because psyq gcc
 * merges those identical clear-and-return tails while leaving every arm its own
 * two-call tail. Every other arm returns. Every arm that leaves a window
 * stores 2 into the shared phase byte D_80143BB0 (0x80143BB0), the two window
 * openers require it to read 2, and four arms wait for the shared gate halfword
 * D_80143C40 (0x80143C40) to read zero. The arms that seed a front selector
 * request read g_ScenarioProgress through one address taken into a local
 * pointer and clear it before they call func_8019FA28; the 0x46 arm reaches the
 * second byte flag bank as the word 0x70 below D_80144F28. Its 0xE6C bytes have
 * no copy in the tree: the table spans in the sibling primary-state handlers of
 * this overlay differ in every case body and every stored literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState15SubstateSequenceScenarioScena0900_801FAE0C(void) {
  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2u) {
      return;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(1u);
    D_80146875 = 1u;
    return;
  case 1:
    if (g_ScenarioProgress != 5u) {
      return;
    }
    func_8015B580(D_8014686C, 0x2D);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 10:
    D_80146875 = 0xBu;
    func_8019FA28(0x25u, 0x200000u, 0x340000u, 0x84u);
    return;
  case 11:
    D_8014832E = 0x1Fu;
    D_80146875 = 0xCu;
    return;
  case 12:
    if (g_ScenarioProgress != 0x19u) {
      return;
    }
    func_801C1E3C(9u);
    func_8015B580(D_8014686C, 0x2F);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 15:
    func_80150224(0xD7u);
    D_80143BB0 = 2u;
    D_80146875 = 0x10u;
    return;
  case 16:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 20:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8015B5A8((u32)&D_80144EB8, 0x3E);
    g_ScenarioProgress = 1u;
    func_801BE1B0(2u);
    D_80146875 = 0x15u;
    return;
  case 21: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 4u) {
      return;
    }
    *progress = 0u;
    D_80146875 = 0x16u;
    func_8019FA28(0x27u, 0x150000u, 0x280000u, 0x88u);
    return;
  }
  case 22:
    if (g_ScenarioProgress != 0xFu) {
      return;
    }
    func_8015B580(D_8014686C, 0x30);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 30:
    func_80150224(0x5Au);
    D_80143BB0 = 2u;
    D_80146875 = 0x10u;
    return;
  case 40:
    if (D_80143BB0 == 2u) {
      return;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(9u);
    D_80146875 = 0x29u;
    return;
  case 41:
    if (g_ScenarioProgress != 7u) {
      return;
    }
    func_8015B580(D_8014686C, 0x31);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 50:
    func_801BE1B0(0xAu);
    D_80146875 = 0x33u;
    return;
  case 51:
    if (g_ScenarioProgress != 5u) {
      return;
    }
    func_8015B580(D_8014686C, 0x33);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 52:
    func_80150224(0xA5u);
    D_80143BB0 = 2u;
    D_80146875 = 0x10u;
    return;
  case 60:
    D_80146875 = 0x3Du;
    func_8019FA28(0x29u, 0x400000u, 0x278000u, 0x89u);
    return;
  case 61:
    D_8014832E = 0x1Fu;
    D_80146875 = 0x3Eu;
    return;
  case 62:
    if (g_ScenarioProgress != 0x1Eu) {
      return;
    }
    func_8015B580(D_8014686C, 0x36);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 70: {
    u8 *flag_bank;

    if (D_801460FB == 3u) {
      return;
    }
    func_80150224(0x87u);
    flag_bank = &D_80144F28;
    D_80143BB0 = 2u;
    D_80146875 = 0x47u;
    func_8015B5A8((u32)flag_bank, 0x45);
    func_8015B580((u32)(flag_bank - 0x70), 0x3E);
    return;
  }
  case 71:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8014ECAC(4u);
    D_80146875 = 0x48u;
    return;
  case 72:
    if (D_80143C40 != 0u) {
      return;
    }
    D_8014832E = 0u;
    D_80146875 = 0x49u;
    return;
  case 73:
    func_80150224(0x88u);
    D_80143BB0 = 2u;
    D_80146875 = 0x4Au;
    return;
  case 74:
    if (D_80143BB0 == 2u) {
      return;
    }
    D_80146875 = 0x4Bu;
    return;
  case 75:
    D_80146875 = 0x4Cu;
    func_8015B580(D_8014686C, 0x34);
    func_8019FA28(0x27u, 0xA0000u, 0x2D8000u, 0x8Bu);
    return;
  case 76:
    func_801C601C(3u);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x4Du;
    return;
  case 77:
    if (g_ScenarioProgress != 4u) {
      return;
    }
    func_801636A0(2u, 1u);
    D_80146875 = 0x4Eu;
    return;
  case 78:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      return;
    }
    func_80161D74(0);
    D_80146875 = 0x4Fu;
    return;
  case 79:
    if (g_ScenarioProgress != 5u) {
      return;
    }
    D_80149333 = 2u;
    D_80146867++;
    D_8014932C += 0x20u;
    if (D_80146867 < 0x14u) {
      return;
    }
    D_8014932C = 0x280u;
    D_80149333 = 2u;
    D_80146867 = 0u;
    D_80146875 = 0x50u;
    return;
  case 80:
    if (g_ScenarioProgress != 7u) {
      return;
    }
    D_80149333 = 2u;
    D_80146867++;
    D_8014932C -= 0x10u;
    if (D_80146867 < 0x28u) {
      return;
    }
    D_8014932C = 0u;
    D_80149333 = 2u;
    D_80146867 = 0u;
    D_80146875 = 0x51u;
    return;
  case 81:
    if (g_ScenarioProgress != 0x1Eu) {
      return;
    }
    D_80149333 = 2u;
    D_80146867++;
    D_8014932C += 0x80u;
    if (D_80146867 < 0xAu) {
      return;
    }
    D_8014932C = 0x500u;
    D_80149333 = 2u;
    D_80146867 = 0u;
    D_80146875 = 0x52u;
    return;
  case 82:
    if (g_ScenarioProgress != 0x1Fu) {
      return;
    }
    D_80149333 = 2u;
    D_80146867++;
    D_8014932C -= 0x80u;
    if (D_80146867 < 0xAu) {
      return;
    }
    D_8014932C = 0u;
    D_80149333 = 2u;
    D_80146867 = 0u;
    D_80146875 = 0x53u;
    return;
  case 83:
    if (g_ScenarioProgress != 0x1Fu) {
      return;
    }
    D_80149333 = 2u;
    D_8014932C = 0u;
    D_80146875 = 0x5Au;
    return;
  case 90:
    if (g_ScenarioProgress != 0x4Eu) {
      return;
    }
    func_8014ECAC(4u);
    D_80146875 = 0x5Bu;
    return;
  case 91:
    if (D_80143C40 != 0u) {
      return;
    }
    g_ScenarioProgress = 0u;
    D_8014832E = 0u;
    D_80146875 = 0x5Cu;
    D_80146258 |= 0x88u;
    func_8015B580(D_8014686C, 0x37);
    func_8019FA28(0x29u, 0x400000u, 0x278000u, 0x8Au);
    D_80143F1F = 0x87u;
    return;
  case 92:
    if (g_ScenarioProgress != 1u) {
      return;
    }
    func_8014ECAC(1u);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x5Du;
    return;
  case 93:
    if (D_80143C40 != 0u) {
      return;
    }
    g_ScenarioProgress = 2u;
    func_801C601C(6u);
    D_80146875 = 0x5Eu;
    return;
  case 94:
    if (g_ScenarioProgress != 5u) {
      return;
    }
    func_8015B580((u32)&D_80144EB8, 0x3E);
    D_80146866 = 1u;
    func_8015B580(D_8014686C, 0x38);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    D_80146258 ^= 8u;
    return;
  case 100:
    func_80150224(0x94u);
    D_80143BB0 = 2u;
    D_80146875 = 0x65u;
    return;
  case 101:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 110:
    if (D_801460FB == 3u) {
      return;
    }
    func_80150224(0x9Cu);
    D_80143BB0 = 2u;
    D_80146875 = 0x6Fu;
    return;
  case 111:
    if (D_80143BB0 == 2u) {
      return;
    }
    D_80146866 = 0u;
    D_80146875 = 0x70u;
    return;
  case 112:
    D_80146875 = 0x71u;
    func_8019FA28(0x29u, 0x40000u, 0x28000u, 0x8Bu);
    return;
  case 113:
    g_ScenarioProgress = 1u;
    D_8014832E = 0x1Fu;
    func_801C601C(7u);
    D_80146875 = 0x72u;
    return;
  case 114:
    if (g_ScenarioProgress != 8u) {
      return;
    }
    D_80146866 = 2u;
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    D_80146258 ^= 8u;
    return;
  case 120:
    if (D_801460FB == 3u) {
      return;
    }
    D_80146866 = 0u;
    D_80146875 = 0x79u;
    func_801BE1B0(2u);
    return;
  case 121:
    g_ScenarioProgress = 1u;
    D_8014832E = 0x1Fu;
    D_80146875 = 0x7Au;
    return;
  case 122: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0xFu) {
      return;
    }
    *progress = 0u;
    D_80146875 = 0x7Bu;
    func_8019FA28(0x64u, 0x4C0000u, 0x338000u, 0x83u);
    return;
  }
  case 123:
    func_801C601C(1u);
    D_80146875 = 0x7Cu;
    return;
  case 124: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0xAu) {
      return;
    }
    *progress = 0u;
    D_80146875 = 0x7Du;
    func_8015B580(D_8014686C, 0x39);
    func_8019FA28(0x64u, 0x4A0000u, 0x1E8000u, 0x84u);
    return;
  }
  case 125: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x14u) {
      return;
    }
    *progress = 0u;
    D_80146875 = 0x7Eu;
    func_8019FA28(0x64u, 0x480000u, 0x340000u, 0x85u);
    return;
  }
  case 126: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 5u) {
      return;
    }
    *progress = 0u;
    D_80146875 = 0x7Fu;
    D_80146258 ^= 0x80u;
    func_8015B580((u32)&D_80144F28, 0x40);
    func_8019FA28(0x3Bu, 0x480000u, 0x2A0000u, 0x82u);
    return;
  }
  case 127:
    if (g_ScenarioProgress != 6u) {
      return;
    }
    func_8015B580(D_8014686C, 0x3A);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 130:
    D_80146875 = 0x83u;
    func_8019FA28(0x3Bu, 0x108000u, 0x140000u, 0x83u);
    return;
  case 131:
    if (g_ScenarioProgress != 7u) {
      return;
    }
    func_8015B580(D_8014686C, 0x22);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  case 140:
    func_801C1DF0(0xDu);
    D_80146875 = 0x8Du;
    D_8014832E = 0u;
    func_8019FA28(0x35u, 0x120000u, 0x180000u, 0x86u);
    return;
  case 141:
    if (g_ScenarioProgress != 1u) {
      return;
    }
    func_8014ECAC(5u);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x8Eu;
    return;
  case 142:
    if (D_80143C40 != 0u) {
      return;
    }
    g_ScenarioProgress = 2u;
    func_801C601C(3u);
    D_80146875 = 0x8Fu;
    return;
  case 143:
    if (g_ScenarioProgress != 7u) {
      return;
    }
    func_8014ECAC(0xDu);
    D_80146875 = 0x90u;
    return;
  case 144:
    if (D_80143C40 != 0u) {
      return;
    }
    D_8014832E = 0u;
    func_801C16B8();
    D_80146875 = 0x91u;
    func_8015D554(0u, 0u, 4u, 0x1Eu);
    return;
  case 145:
    func_801636A0(0u, 1u);
    D_80146875 = 0x92u;
    return;
  case 146:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      return;
    }
    D_80146875 = 0x93u;
    return;
  case 147:
    func_80150224(0x37u);
    D_80143BB0 = 2u;
    D_80146875 = 0x94u;
    return;
  case 148: {
    u8 *phase = &D_80143BB0;

    if (*phase == 2u) {
      return;
    }
    *phase = 6u;
    D_80146875 = 0x95u;
    return;
  }
  case 149:
    if (D_80143BB0 != 0u) {
      return;
    }
    D_80146875 = 0x96u;
    func_8015B580(D_8014686C, 0x23);
    func_8019FA28(0x2Du, 0x230000u, 0x240000u, 4u);
    return;
  case 150:
    func_8015B580((u32)&D_80144F28, 0x33);
    D_8014832E = 0x1Fu;
    func_8015C058();
    g_ScenarioProgress = 0u;
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    return;
  default:
    return;
  }
}
