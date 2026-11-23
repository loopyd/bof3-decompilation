#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80146258;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_1F800000;
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern u16 D_801492D8;
extern u16 D_801492DA;

void func_8015C058(void);
void func_8015B580(u32 arg0, s32 arg1);
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015D404(u32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C1E3C(u8 index);
void func_801C601C(u32 arg0);
u8 func_8019601C(void);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);

/* @source 0x801F8C78
 * @behavior Slot 12 of the 31-word code-pointer run at 0x801FE288 (pointer
 *   word at 0x801FE2B8) that callProgressChainHandler
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot; no jal of the payload names 0x801F8C78 and that
 *   table word is its only reference. It switches the shared sub-state byte
 *   D_80146875 (0x80146875, read with lbu before the frame is grown) through
 *   the eight-word absolute case table at 0x801F6D54, bounded by sltiu against
 *   8, so sub-states 0 through 7 select a case and any larger value returns
 *   through the single epilogue. Sub-state 0 probes flag 0x35 of the shared
 *   flag word D_8014686C (0x8014686C) and plays effect (0, 1) when that probe
 *   reports zero and effect (0, 0) afterwards, then runs func_801BE1B0(0x18),
 *   func_801C601C(5) and advances to 1. Sub-states 1 and 2 seed one 0x74-byte
 *   front-end record at 0x80143FC8 from the object byte func_8019601C(), which
 *   they keep in scratchpad 0x1F800000: row byte 0 takes 1, +5 takes 0x13, +9
 *   takes 0x10 and the s32 words at +0x64 and +0x68 take the signed halfwords
 *   D_801492D8 and D_801492DA, while +0x6C takes 0x2C6 in sub-state 1 and 0x1FE
 *   in sub-state 2, which only runs on progress 0xA2 and then records the
 *   seeded slot in D_80146867 and advances to 3; an object byte of 0xFF skips
 *   the seeding and returns. Sub-state 3 waits for that record's row byte 0 to
 *   clear, then zeroes D_80146867, runs func_801BE1B0(0xA) and advances to 4.
 *   Sub-state 4, on progress 0x63, raises mode bit 8 of the shared halfword
 *   D_80146258 (0x80146258), queues cue func_80161C20(0x20, 0x64, 8), advances
 *   to 5 and raises mode bit 0x80 of D_80146258. Sub-state 5, on progress 0x71,
 *   runs func_801C601C(1) and advances to 6. Sub-state 6, on progress 0x7E,
 *   clears the effect-bank byte D_80146866, flips mode bit 8, runs
 *   func_801C1E3C(0), advances to 7, flips mode bit 0x80 and seeds the front
 *   selector context func_8019FA28(0x1B, 0x220000, 0x260000, 0x80). Sub-state
 *   7 sets flag bits 0x3F of D_8014686C through func_8015B580, releases the
 *   overlay through func_8015C058, stores 1 in D_80146866 and returns the
 *   sub-state and primary-state bytes D_80146875 and D_80146874 (0x80146874)
 *   to zero. It takes no arguments and returns nothing; its 0x18-byte frame
 *   keeps only $ra, and the free-slot probe, the seeding row layout and the
 *   flag and mode-flag helpers leave their own roles unknown.
 *
 * Name evidence: no in-payload jal targets 0x801F8C78, and the only payload
 * word equal to that address is the chain-table pointer at 0x801FE2B8, so the
 * span is a progress-chain step handler and takes a target-qualified step name.
 * The walk is the shape of the exact sibling
 * runLadderToggleRelease
 * (0x801F9678, slot 15 of the same run), which switches the same sub-state byte
 * through an absolute table, raises and flips mode bits of D_80146258, sets
 * flag bits of D_8014686C through func_8015B580 and releases the overlay
 * through func_8015C058, and of the exact sibling
 * runLadderContextBits
 * (0x801F9134, slot 13), which clears the same D_80146867 effect-bank byte
 * before zeroing D_80146875 and D_80146874. The record seeding block is the
 * shape of the scena00 span func_801F9BE4 (0x801F9BE4, exact) case 6, whose
 * object probe, scratchpad store, 0xFF skip and 0x74-byte rows match this
 * span's; every address, literal and shift count here is this span's own. The
 * free-slot probe, the record row values 0x2C6 and 0x1FE, the progress gates
 * 0xA2, 0x63, 0x71 and 0x7E, the selector-context bundle and the flag bits are
 * this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801F8C78-0x801F9133 is one
 * complete function of 0x4BC bytes with a single jr $ra epilogue, so it is
 * neither data nor a fragment and no data or carve lever applies. The dispatch
 * table is eight words, 0x801F6D54 through 0x801F6D73, the eleven-word table of
 * the sibling at 0x801F9134 beginning immediately at 0x801F6D74. Clone is
 * declined on payload evidence: a scan of all 398 payloads under out/binaries
 * (0x4D2709 bytes) for these 1212 bytes matched this span alone, so no
 * byte-identical sibling body exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runLadderRowsToggle(void) {
  switch (D_80146875) {
  case 0:
    if (func_8015B5D4(D_8014686C, 0x35) == 0) {
      func_8015D404(0, 1);
    }
    func_8015D404(0, 0);
    func_801BE1B0(0x18);
    func_801C601C(5);
    D_80146875 = 1;
    break;
  case 1: {
    u8* scratch;
    u8  object_index;

    object_index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = object_index;

    if (object_index != 0xffu) {
      D_80143FC8[(u32)object_index * 0x74u] = 1u;
      D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
      *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = (s32)(s16)D_801492D8;
      *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
      *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x2C6;
      D_80143FD1[(u32)*scratch * 0x74u] = 0x10u;
      D_80146875 = 2u;
    }

    break;
  }
  case 2: {
    u8* scratch;
    u8  object_index;

    if (g_ScenarioProgress != 0xA2u) {
      break;
    }

    object_index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = object_index;

    if (object_index != 0xffu) {
      D_80143FC8[(u32)object_index * 0x74u] = 1u;
      D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
      *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = (s32)(s16)D_801492D8;
      *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
      *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x1FE;
      D_80143FD1[(u32)*scratch * 0x74u] = 0x10u;
      D_80146875 = 3u;
      D_80146867 = *scratch;
    }
    break;
  }
  case 3: {
    u8* row_slot;

    row_slot = &D_80146867;

    if (D_80143FC8[(u32)*row_slot * 0x74u] == 0u) {
      *row_slot = 0;
      func_801BE1B0(0xA);
      D_80146875 = 4u;
    }
    break;
  }
  case 4:
    if (g_ScenarioProgress == 0x63u) {
      D_80146258 |= 8;
      func_80161C20(0x20u, 0x64, 8);
      D_80146875 = 5u;
      D_80146258 |= 0x80;
    }
    break;
  case 5:
    if (g_ScenarioProgress == 0x71u) {
      func_801C601C(1);
      D_80146875 = 6u;
    }
    break;
  case 6:
    if (g_ScenarioProgress == 0x7Eu) {
      D_80146866 = 0;
      D_80146258 ^= 8;
      func_801C1E3C(0);
      D_80146875 = 7u;
      D_80146258 ^= 0x80;
      func_8019FA28(0x1B, 0x220000, 0x260000, 0x80);
    }
    break;
  case 7:
    func_8015B580(D_8014686C, 0x3F);
    func_8015C058();
    D_80146866 = 1u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
}
