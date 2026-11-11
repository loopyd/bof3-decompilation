#include "bof3/battle/battle03_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */
void func_800AAEBC(s16 arg0, u8 arg1); /* @source 0x800AAEBC */
void func_8009D198(void); /* @source 0x8009D198 */
s32  func_80165F68(u8 arg0, u8 arg1); /* @source 0x80165F68 */
void func_800AB834(void); /* @source 0x800AB834 */
void func_800AB910(void); /* @source 0x800AB910 */
void func_8009BBE8(void); /* @source 0x8009BBE8 */

/* Subject cells the pass publishes for the selection handlers that run after
 * it: 0x8014639C holds the scratch work pointer, 0x801463A0 the subject
 * halfword pair inside the current enemy work record at +0xF4 and 0x80146394
 * the scratch work selector byte. Each address is bound as its own symbol
 * because the original reaches it as its own displacement (`sw %lo(D_8014639C)`,
 * `lw %lo(D_801463A0)`). The 0x801463A0 cell is volatile: the original reloads
 * it for the +0x06 and +0x08 clears and for the resolved damage halfword
 * instead of forwarding the pointer it just stored. */
extern u8*  D_8014639C;   /* @source 0x8014639C @kind data */
extern s16* volatile D_801463A0; /* @source 0x801463A0 @kind data */
extern volatile u8 D_80146394; /* @source 0x80146394 @kind unknown */
/* Damage-gate halfwords the status handlers accumulate into; this pass folds
 * 0x801463D8 into the enemy work flags at +0x82, clears it and hands the cell
 * to func_8009BBE8, and clears the companion 0x801463DA gate. */
extern u16  D_801463D8; /* @source 0x801463D8 @kind unknown */
extern u16  D_801463DA; /* @source 0x801463DA @kind unknown */
/* In-payload destination of the 0x20-byte subject block copy. */
extern u8   D_801EC2E8[]; /* @source 0x801EC2E8 @kind data */

/* Byte view of the 0x20-byte block the pass snapshots out of the current enemy
 * work record at +0xA0: every member is a byte, so the copy keeps the
 * original's unaligned lwl/lwr + swl/swr word moves instead of aligned lw/sw
 * pairs. */
typedef struct Battle03EnemySubjectBlock {
  u8 bytes_00[0x20];
} Battle03EnemySubjectBlock;

/* @source 0x801E3C14
 * @behavior Enemy selection pass entry 0 of the scratch work-byte-3 table
 * 0x801EB424: publishes the scratch work pointer cell 0x8014639C, the enemy
 * work subject halfword cell 0x801463A0 (record +0xF4) and the scratch work
 * selector byte 0x80146394, clears the record halfword +0xF8, the subject bytes
 * +0x06/+0x08 and the damage gates 0x801463D8/0x801463DA, then copies the
 * record block +0xA0 to 0x801EC2E8. It selects the action through
 * the battle mode byte 0x80146375: mode 1 resolves the damage through
 * applyResolvedDamageToTarget for actor byte 0x80146374 and target byte
 * 0x80146384 and stores the halfword in the subject +0x04, while modes 4 and 5
 * call func_8009D198. While the 0x801462EA byte is set it runs the current
 * enemy work handler +0xE4 with argument 1. A record byte +0xFC bit 1 with a
 * zero +0xF8 returns early after storing 3 into scratch work byte +0x02. While
 * record word +0x100 has no 0x200 bit the pass raises the enemy script byte
 * helper func_801E2314 with class 4 for mode 1, for mode 5 when the
 * func_80165F68 result for the 0x801463C0 halfword has no 0x04 bit, and for
 * mode 4 when the 0x801CA718 kind-keyed byte (+0x14 stride) has no 0x04 bit. It
 * replays the pending halves +0xF8/+0xFA through func_800AAC88/func_800AAEBC
 * gated by record byte +0xFC bits 1 and 2, ORs the accumulated 0x801463D8 gate
 * into record halfword +0x82, clears that gate cell and calls func_8009BBE8.
 * For a positive +0xF8 it clears scratch work byte +0x09 and, when the
 * 0x801462E8 halfword carries bit 0x80,
 * calls func_8019651C on the scratch work with 0x10/0x10/0x10/0, then raises
 * record bit 0x4000; a nonzero +0x94 instead submits the work effect variant
 * of the +0xE8 effect halfword while 0x801462EA is set. A negative +0xF8 or
 * +0xFA raises front-end cue 0x206. Finally, while the 0x801462E8 halfword has
 * no 0x2000 bit, mode 1 and mode 4 with the 0x801CA71C kind mask bit 0x800
 * call func_800AB834/func_800AB910, and the pass clears the 0x801462E8 bit
 * 0x80, runs enemyReadyOrHelper1 and advances scratch work byte +0x03.
 * @status partial
 * @match 98.10
 * @residual 310/316 instructions matched at 1264 bytes. From +0x0008 the
 * opening block loads the current enemy work cell into a0 instead of v1 and
 * the selector into v1 instead of a0. The selector load follows the scratch
 * pointer publish, and the record +0xF8 clear follows the two subject clears.
 * The remainder of the body matches. The two subject clears use volatile
 * field views to retain the original ordered pointer reloads and stores.
 */
void resolveEnemySelectionDamageAndSubmitPendingEffects(void) {
  Battle03LocalWork* work;
  u16*               gate;
  u16                kind;
  u8*                mode;
  u8                 selector;
  Battle03EnemyWork* record;

  work = D_1F800044;
  record = battleCurrentEnemyWorkState;
  D_801463DA = 0;
  D_801463D8 = 0;
  D_8014639C = (u8*)work;
  selector = work->unk_05;
  D_801463A0 = (s16*)((u8*)record + 0xF4);
  D_80146394 = selector;
  ((volatile s16*)D_801463A0)[3] = 0;
  *(volatile u8*)(D_801463A0 + 4) = 0;
  record->unk_f8 = 0;
  *(Battle03EnemySubjectBlock*)D_801EC2E8 =
      *(Battle03EnemySubjectBlock*)((u8*)battleCurrentEnemyWorkState + 0xA0);
  switch (D_80146375) {
  case 1u:
    D_801463A0[2] =
        applyResolvedDamageToTarget(BATTLE_GLOBAL_BYTE_6374, D_80146384);
    break;
  case 4u:
  case 5u:
    func_8009D198();
    break;
  default:
    break;
  }
  if (BATTLE_GLOBAL_BYTE_62EA != 0u) {
    battleCurrentEnemyWorkState->unk_e4(1);
  }
  if (((battleCurrentEnemyWorkState->unk_fc & 1u) != 0u) &&
      ((s16)battleCurrentEnemyWorkState->unk_f8 == 0)) {
    D_1F800044->unk_02 = 3;
    return;
  }
  if ((battleCurrentEnemyWorkState->unk_100 & 0x200u) == 0u) {
    mode = (u8*)&BATTLE_GLOBAL_BYTE_6375;
    if (*mode == 1u) {
      func_801E2314(4);
    }
    if (*mode == 5u) {
      kind = BATTLE_GLOBAL_HALF_63C0;
      if ((func_80165F68(kind >> 8, kind & 0xFFu) & 4) == 0) {
        func_801E2314(4);
      }
    }
    if (D_80146375 == 4u) {
      if ((D_801CA718[(u32)BATTLE_GLOBAL_HALF_63C0 * 0x14u] & 4u) == 0u) {
        func_801E2314(4);
      }
    }
  }
  if ((battleCurrentEnemyWorkState->unk_fc & 1u) != 0u) {
    func_800AAC88((s16)battleCurrentEnemyWorkState->unk_f8,
                  D_1F800044->unk_05);
  }
  if ((battleCurrentEnemyWorkState->unk_fc & 2u) != 0u) {
    func_800AAEBC((s16)battleCurrentEnemyWorkState->unk_fa,
                  D_1F800044->unk_05);
  }
  gate = &D_801463D8;
  battleCurrentEnemyWorkState->unk_82 |= *gate;
  *gate = 0;
  func_8009BBE8();
  if ((s16)battleCurrentEnemyWorkState->unk_f8 > 0) {
    if (battleCurrentEnemyWorkState->unk_94 == 0u) {
      D_1F800044->unk_09 = 0;
      if ((BATTLE_GLOBAL_HALF_62E8 & 0x80u) != 0u) {
        func_8019651C(D_1F800044, 0x10, 0x10, 0x10, 0);
      }
      battleCurrentEnemyWorkState->unk_82 = 0x4000u;
    } else if (BATTLE_GLOBAL_BYTE_62EA != 0u) {
      submitEffectUnlessNeg1(battleCurrentEnemyWorkState->unk_e8[2]);
    }
  }
  if (((s16)battleCurrentEnemyWorkState->unk_f8 < 0) ||
      ((s16)battleCurrentEnemyWorkState->unk_fa < 0)) {
    func_8015DF18(0x206u);
  }
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x2000u) == 0u) {
    if (D_80146375 == 1u) {
      func_800AB834();
      func_800AB910();
    }
    if (D_80146375 == 4u) {
      if ((D_801CA71C[BATTLE_GLOBAL_HALF_63C0].mask_00 & 0x800u) != 0u) {
        func_800AB834();
        func_800AB910();
      }
    }
  }
  *(u16*)&BATTLE_GLOBAL_HALF_62E8 &= 0xFF7Fu;
  enemyReadyOrHelper1();
  D_1F800044->unk_03++;
}
