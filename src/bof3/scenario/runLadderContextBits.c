#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C058(void);
void func_8015C088(void);
void func_8015B580(u32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);

/* @source 0x801F9134
 * @behavior Slot 13 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2B4) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot; no jal of the payload names 0x801F9134 and that
 *   table word is its only reference. It switches the shared sub-state byte
 *   D_80146875 (0x80146875, read with lbu before the frame is grown) through
 *   the eleven-word absolute case table at 0x801F6D74, bounded by sltiu
 *   against 0xB, so sub-states 0 through 0xA select a case and any larger value
 *   returns through the single epilogue. Sub-states 2 and 4 have no body: their
 *   table words hold the epilogue address. Sub-state 0 runs the shared helper
 *   func_8015C088 and then func_801BE1B0 with 3, and advances to 1. Sub-state
 *   1, once the shared progress byte g_ScenarioProgress (0x80146864) reads 1,
 *   stores 4 in the effect-bank byte D_80146866, advances to 3, raises mode bit
 *   8 of the shared halfword D_80146258 (0x80146258) with lhu/ori/sh and seeds
 *   the front selector context func_8019FA28(0x1B, 0x400000, 0x1B0000, 0x94).
 *   Sub-state 3, once that byte reads 9, sets flag bits 0xF of the shared flag
 *   word D_8014686C (0x8014686C) through func_8015B580, flips mode bit 8 of
 *   D_80146258 with lhu/xori/sh, releases the overlay through func_8015C058,
 *   clears D_80146865, D_80146866 and D_80146867 and returns the sub-state and
 *   primary-state bytes D_80146875 and D_80146874 (0x80146874) to zero.
 *   Sub-state 5 runs func_801BE1B0 with 4 and advances to 6. Sub-state 6, once
 *   the progress byte reads 1, runs func_801C601C with 0 and advances to 7.
 *   Sub-state 7, once that byte reads 2, hands 0x200000, 0x260000 and 8 to
 *   func_801BFE34 and advances to 8. Sub-state 8, once that byte reads 0x10,
 *   runs func_801C7A54 with 8 and advances to 9. Sub-state 9, once that byte
 *   reads 0x11, runs func_801BE1B0 with 0x12 and advances to 0xA. Sub-state 0xA,
 *   once that byte reads 0x14, sets flag bit 0x10 of D_8014686C through
 *   func_8015B580 and takes the same release tail through a shared copy of it.
 *   It takes no arguments and returns nothing; its 0x18-byte frame keeps only
 *   $ra, the sub-state byte is read once before the frame is grown, and each
 *   progress gate re-reads the byte through its own address. The roles of the
 *   helper modes, of the mode bit 8 and of the cleared bytes remain unknown.
 *
 * Name evidence: no in-payload jal targets 0x801F9134, and the only payload
 * word equal to that address is the chain-table pointer at 0x801FE2B4, so the
 * span is a progress-chain step handler and takes a target-qualified step name.
 * The walk is the shape of the exact sibling
 * runLadderToggleRelease
 * (0x801F9678, slot 15 of the same run), which switches the same sub-state byte
 * through an absolute table, gates every case on the progress byte, sets a flag
 * bit of D_8014686C through func_8015B580, flips mode bit 8 of D_80146258,
 * releases the overlay through func_8015C058 and clears the shared bytes
 * 0x80146865 through 0x80146867 before zeroing D_80146875 and D_80146874, and
 * of the exact slot-14 sibling
 * runCompletionSubstatesSeeding
 * (0x801F936C), which seeds the same front selector context through
 * func_8019FA28 and carries the same effect-bank byte D_80146866. The helper
 * modes, the progress gates, the flag bits 0xF and 0x10, the selector-context
 * bundle and the cleared bytes are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801F9134-0x801F936B is one
 * complete function of 0x238 bytes with a single jr $ra epilogue, so it is
 * neither data nor a fragment and no data or carve lever applies. The dispatch
 * table is 11 words, 0x801F6D74 through 0x801F6D9F, and word 11 would sit at
 * 0x801F6DA0, a zero word that is not a case address. Clone is declined on
 * payload evidence: a scan of every payload under out/binaries for these 568
 * bytes matched the span alone, and a scan of the same payloads for the opcode
 * sequence of the 142 instructions matched the span alone as well, so no
 * byte-identical or relocation-different sibling body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runLadderContextBits(void) {
  switch (D_80146875) {
  case 0:
    func_8015C088();
    func_801BE1B0(3);
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress == 1) {
      D_80146866 = 4;
      D_80146875 = 3;
      D_80146258 |= 8;
      func_8019FA28(0x1B, 0x400000, 0x1B0000, 0x94);
    }
    break;
  case 3:
    if (g_ScenarioProgress == 9) {
      func_8015B580(D_8014686C, 0xF);
      D_80146258 ^= 8;
      func_8015C058();
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  case 5:
    func_801BE1B0(4);
    D_80146875 = 6;
    break;
  case 6:
    if (g_ScenarioProgress == 1) {
      func_801C601C(0);
      D_80146875 = 7;
    }
    break;
  case 7:
    if (g_ScenarioProgress == 2) {
      func_801BFE34(0x200000, 0x260000, 8);
      D_80146875 = 8;
    }
    break;
  case 8:
    if (g_ScenarioProgress == 0x10) {
      func_801C7A54(8);
      D_80146875 = 9;
    }
    break;
  case 9:
    if (g_ScenarioProgress == 0x11) {
      func_801BE1B0(0x12);
      D_80146875 = 0xA;
    }
    break;
  case 0xA:
    if (g_ScenarioProgress == 0x14) {
      func_8015B580(D_8014686C, 0x10);
      func_8015C058();
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  }
}
