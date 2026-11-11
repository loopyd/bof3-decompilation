#include "bof3/battle/battle03_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */
void func_800AAEBC(s16 arg0, u8 arg1); /* @source 0x800AAEBC */
void func_8009D198(void); /* @source 0x8009D198 */
s32  func_80165F68(u8 arg0, u8 arg1); /* @source 0x80165F68 */
void func_800AB834(void); /* @source 0x800AB834 */
void func_800AB910(void); /* @source 0x800AB910 */

/* Selection-pass scratch cells. 0x8014639C holds the active scratch work
 * pointer, 0x801463A0 the selection halfword pair inside the local work record
 * at +0x118 and 0x80146394 the actor byte the pass resolves damage for; each
 * address is bound as its own symbol because the original reaches it as its own
 * displacement (`sw %lo(D_8014639C)`, `lw %lo(D_801463A0)`). The 0x801463A0
 * cell is volatile: the original reloads it for the +0x06 and +0x08 clears
 * instead of forwarding the pointer it just stored. */
extern u8*  D_8014639C;   /* @source 0x8014639C @kind data */
extern s16* volatile D_801463A0; /* @source 0x801463A0 @kind data */
extern volatile u8 D_80146394; /* @source 0x80146394 @kind unknown */
/* Damage-gate halfwords the status handlers accumulate into; this pass folds
 * 0x801463D8 into the local work record flags at +0x80 and clears it, and
 * 0x801463DA is the companion target-status gate the same handlers raise. */
extern u16  D_801463D8;   /* @source 0x801463D8 @kind unknown */
extern u16  D_801463DA;   /* @source 0x801463DA @kind unknown */
/* In-payload destination of the 0x20-byte selection block copy above the
 * 0x801EC2EE cue halfword. */
extern u8   D_801EC2E8[]; /* @source 0x801EC2E8 @kind data */

/* Byte view of the 0x20-byte selection block the pass snapshots out of the
 * local work record: every member is a byte, so the copy keeps the original's
 * unaligned lwl/lwr + swl/swr word moves instead of aligned lw/sw pairs. */
typedef struct Battle03SelectionBlock {
  u8 bytes_00[0x20];
} Battle03SelectionBlock;

/* @source 0x801DFCB4
 * @behavior Local selection pass entry 0 of the scratch work-byte-3 table:
 * publishes the scratch work pointer cell 0x8014639C, the local work record
 * selection halfword pair cell 0x801463A0 and the actor byte 0x80146394,
 * clears the record halfword +0x11C and the selection bytes +0x06/+0x08 behind
 * 0x801463A0 together with the damage gates 0x801463D8/0x801463DA, copies the
 * record block +0x90..+0xAF to 0x801EC2E8, raises 0x801EC2EE when record
 * halfword +0x80 carries bit 0x800, and advances the 0x801EC303 byte by 0x19 up
 * to 0x64 while record word +0x124 carries bit 1. It then selects the action
 * through the battle mode byte 0x80146375: mode 1 resolves the damage through
 * applyResolvedDamageToTarget for actor byte 0x80146374 and stores it in the
 * selection halfword +0x04, modes 4 and 5 call func_8009D198. A record byte
 * +0x120 bit 1 with a zero +0x11C returns early after storing 3 into scratch
 * work byte +0x02. While record word +0x124 has no 0x200 bit the pass forwards
 * the selected effect id for mode 1, and for modes 4/5 only when the mode 4
 * kind-keyed byte 0x801CA718 (+0x14 stride) or the mode 5 func_80165F68 result
 * has no 0x04 bit. It replays the pending effect halves +0x11C/+0x11E through
 * func_800AAC88/func_800AAEBC gated by record byte +0x120 bits 1 and 2, ORs the
 * accumulated 0x801463D8 gate into record halfword +0x80 and clears it. For a
 * positive +0x11C it submits work effect variant 0 when +0x88 is nonzero, or
 * else raises record bit 0x4000 (from the +0x128 bit 2 test), submits variant 1
 * while +0x128 has no bit 2 and runs func_80196718 on the scratch work; a
 * negative +0x11C or +0x11E raises front-end cue 0x206. Finally, while the
 * battle global halfword 0x801462E8 has no 0x2000 bit, mode 1 and mode 4 with
 * the 0x801CA71C kind mask bit 0x800 call func_800AB834/func_800AB910 and the
 * pass clears the 0x801462E8 bit 0x80, runs localReadyOrHelper1 and advances
 * scratch work byte +0x03.
 * @status partial
 * @match 73.33
 * @residual 231/314 instructions matched, 1256 -> 1260 bytes (315 current
 * instructions). From +0x0000 the
 * first two prologue loads come out in the opposite order: this source loads
 * the volatile 0x80146250 record cell first into a3 and the 0x1F800044 scratch
 * work pointer second into a0, while the original loads the scratch pointer
 * into v0 and the record cell into v1, and that ordering carries the register
 * web through the body. The shared header declares the 0x80146250 cell
 * volatile, so its loads cannot be moved below the scratch load, and every
 * source ordering tried (record assigned before or after the gate stores, with
 * or without the scratch local) compiles to the same schedule. Further
 * residuals: the 0x801EC303 clamp re-materializes its sum in the branch delay
 * slot in the original, the mode-5 arm of the second switch sits out of line
 * there, and this source keeps the second 0x801463A0 load hoisted above the
 * selection stores.
 */
void resolveSelectionDamageAndSubmitPendingEffects(void) {
  Battle03LocalWork* work;
  Battle03LocalWork* record;
  u16*               gate;
  u16                kind;
  s32                next;

  work = D_1F800044;
  record = D_80146250;
  D_801463DA = 0;
  D_801463D8 = 0;
  D_80146394 = work->unk_05;
  D_8014639C = (u8*)work;
  D_801463A0 = (s16*)((u8*)record + 0x118);
  record->unk_11c = 0;
  D_801463A0[3] = 0;
  *(u8*)(D_801463A0 + 4) = 0;
  *(Battle03SelectionBlock*)D_801EC2E8 =
      *(Battle03SelectionBlock*)((u8*)D_80146250 + 0x90);
  record = D_80146250;
  if ((record->unk_80 & 0x800u) != 0u) {
    D_801EC2EE = 1;
  }
  if ((record->unk_124 & 1u) != 0u) {
    next = D_801EC303 + 0x19;
    if (next >= 0x64) {
      next = 0x64;
    }
    D_801EC303 = (u8)next;
  }
  switch (*(u8*)&BATTLE_GLOBAL_BYTE_6375) {
  case 1u:
    D_801463A0[2] =
        applyResolvedDamageToTarget(BATTLE_GLOBAL_BYTE_6374, D_80146394);
    break;
  case 4u:
  case 5u:
    func_8009D198();
    break;
  default:
    break;
  }
  record = D_80146250;
  if (((record->unk_120 & 1u) != 0u) && ((s16)record->unk_11c == 0)) {
    D_1F800044->unk_02 = 3;
    return;
  }
  record = D_80146250;
  if ((record->unk_124 & 0x200u) == 0u) {
    switch (*(u8*)&BATTLE_GLOBAL_BYTE_6375) {
    case 1u:
      forwardSelectedEffectId();
      break;
    case 4u:
      if ((D_801CA718[(u32)BATTLE_GLOBAL_HALF_63C0 * 0x14u] & 4u) == 0u) {
        forwardSelectedEffectId();
      }
      break;
    case 5u:
      kind = BATTLE_GLOBAL_HALF_63C0;
      if ((func_80165F68(kind >> 8, kind & 0xFFu) & 4) == 0) {
        forwardSelectedEffectId();
      }
      break;
    default:
      break;
    }
  }
  record = D_80146250;
  if ((record->unk_120 & 1u) != 0u) {
    func_800AAC88((s16)record->unk_11c, D_1F800044->unk_05);
    record = D_80146250;
  }
  if ((record->unk_120 & 2u) != 0u) {
    func_800AAEBC((s16)record->unk_11e, D_1F800044->unk_05);
  }
  gate = &D_801463D8;
  record = D_80146250;
  record->unk_80 |= *gate;
  *gate = 0;
  if ((s16)record->unk_11c > 0) {
    if (record->unk_88 == 0u) {
      if ((record->unk_128 & 2u) != 0u) {
        record->unk_80 |= 0x4000u;
      } else {
        record->unk_80 = 0x4000u;
      }
      record = D_80146250;
      if ((record->unk_128 & 2u) == 0u) {
        submitCurrentWorkEffectVariant(1u);
      }
      func_80196718(D_1F800044);
    } else {
      submitCurrentWorkEffectVariant(0u);
    }
  }
  record = D_80146250;
  if (((s16)record->unk_11c < 0) || ((s16)record->unk_11e < 0)) {
    func_8015D71C(0x206u);
  }
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x2000u) == 0u) {
    if (*(u8*)&BATTLE_GLOBAL_BYTE_6375 == 1u) {
      func_800AB834();
      func_800AB910();
    }
    if (*(u8*)&BATTLE_GLOBAL_BYTE_6375 == 4u) {
      if ((D_801CA71C[(u32)BATTLE_GLOBAL_HALF_63C0].mask_00 & 0x800u) != 0u) {
        func_800AB834();
        func_800AB910();
      }
    }
    BATTLE_GLOBAL_HALF_62E8 &= 0xFF7Fu;
  }
  localReadyOrHelper1();
  D_1F800044->unk_03++;
}
