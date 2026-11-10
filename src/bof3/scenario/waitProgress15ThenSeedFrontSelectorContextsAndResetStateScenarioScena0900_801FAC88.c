#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80143C40;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;

/* Shared front-end mode setter at 0x8014ECAC. */
void func_8014ECAC(u16 arg0);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end request setter at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared field-record reinitializer at 0x801C16B8, a no-argument overlay entry
 * that the field controllers run to rebuild every record from the work block. */
void func_801C16B8(void);
/* Shared two-argument controller-step helper at 0x801636A0. */
void func_801636A0(u32 arg0, u32 arg1);
/* Shared record-state query at 0x80163EA0, returning the live record byte
 * zero-extended into a word. */
u32 func_80163EA0(void);

/* @source 0x801FAC88
 * @behavior Word 14 (the word stored at 0x801FE424) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets this address and that table word is its
 * only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the fifteen-entry absolute jump table jtbl_801F702C_main
 * at 0x801F702C, an overlay-local table in this payload leading tabledata blob,
 * and it stores every shared byte directly: the original re-materialises the
 * shared base with a fresh lui at for each store, so the seed keeps one extern
 * per cell and no local pointer, and its 0x18-byte frame keeps only $ra.
 * Sub-state 0 advances the sub-state byte to 1 and requests front selector
 * context seed 0x35 with the two 16.16 context words 0x3D8000 and 0xB0000 and
 * kind 0x85 through func_8019FA28. Sub-state 1, once the shared scenario
 * progress byte g_ScenarioProgress (0x80146864) reads 0xF, runs the shared
 * front-end mode setter func_8014ECAC(0xD), the shared field-record
 * reinitializer func_801C16B8 and the shared step helper func_801636A0(0, 1),
 * then advances the sub-state byte to 2. Sub-state 2, while the shared gate
 * halfword D_80143C40 (0x80143C40, read with lhu) reads 0, clears the effect
 * bank byte D_8014832E (0x8014832E) and, unless the record-state query
 * func_80163EA0 returns a zero low byte, advances to 3. Sub-state 3 advances to
 * 4 and requests front selector context seed 0x41 with the context words
 * 0x290000 and 0x320000 and kind 4 through func_8019FA28. Sub-state 4 sets
 * scenario flag bit 0x2E of the shared flag word D_8014686C (0x8014686C, read
 * with lw) through func_8015B580, arms the effect bank byte to 0x1F, toggles
 * bit 0x0E of the shared mode halfword D_80146258 (0x80146258, read with lhu),
 * runs the shared release helper func_8015C058 and then clears the scenario
 * progress byte, the three shared bytes 0x80146865, 0x80146866 and 0x80146867,
 * the sub-state byte and the primary state byte D_80146874 back to zero, which
 * ends the state. Sub-states 5 through 14 are the remaining table entries, all
 * pointing at this body exit label, and return without touching the tail. It
 * takes no arguments, reads no other state and writes no other byte. Its 0x184
 * bytes have no copy in the tree: an exact-span search over every payload under
 * out/binaries finds no second copy of the 0x184-byte body, and the only
 * overlay-local absolute word in it is the jump-table base 0x801F702C, which
 * every other overlay would carry at its own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitProgress15ThenSeedFrontSelectorContextsAndResetStateScenarioScena0900_801FAC88(void) {
  switch (D_80146875) {
  case 0:
    D_80146875 = 1u;
    func_8019FA28(0x35, 0x3D8000, 0xB0000, 0x85);
    break;
  case 1:
    if (g_ScenarioProgress != 0xFu) {
      break;
    }
    func_8014ECAC(0xD);
    func_801C16B8();
    func_801636A0(0, 1);
    D_80146875 = 2u;
    break;
  case 2:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    D_80146875 = 3u;
    break;
  case 3:
    D_80146875 = 4u;
    func_8019FA28(0x41, 0x290000, 0x320000, 4);
    break;
  case 4:
    func_8015B580(D_8014686C, 0x2E);
    D_8014832E = 0x1Fu;
    D_80146258 ^= 0x0Eu;
    func_8015C058();
    g_ScenarioProgress = 0u;
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
    break;
  }
}
