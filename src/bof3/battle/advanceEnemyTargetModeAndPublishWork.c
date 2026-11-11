#include "bof3/battle/battle03_internal.h"

extern int rand(void);

/* @source 0x801E25E0
 * @behavior Derives the enemy target mode byte 0x80146375 from the two-bit
 * field that the target mode pack table 0x800B51F8 holds for record byte +0x7E
 * and the low rand bits, and forces it to 0 while 0x801462E8 bit 0x4000 is set
 * and record halfword +0xAA stays below 2. It then advances that mode through
 * 0..4 using the record halfword +0x80 readiness bits 0x80, 0x40 and 0x20,
 * seeding record halfword +0xF6 from the eight-byte candidate run at +0x8C when
 * the kind path opens. A record whose halfword +0x82 holds bit 0x20, or whose
 * word +0x104 holds bit 0x4000, keeps mode 1 and picks a target through
 * pickTargetStoreGlobal; otherwise func_801E2A88 supplies the selected target
 * byte 0x80146384. While the battle mode byte 0x801462EA is set it also
 * publishes the record as the current enemy work (0x801EB4E8 and scratchpad
 * +0x44) and runs its +0xE4 handler. It ends by copying the mode into record
 * byte +0xF5, or by clearing both the mode and +0xF5 when the selected target
 * byte reads 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceEnemyTargetModeAndPublishWork(u8 arg0) {
  Battle03EnemyWork* work;
  /* The original reaches the mode byte through a const pointer, so the address
   * is rematerialized at every site and each mode store is a single instruction
   * that can fill a branch delay slot. */
  u8* const mode = (u8*)&BATTLE_GLOBAL_BYTE_6375;

  *mode =
      (D_800B51F8[D_801EB630[arg0].unk_7e] >> ((rand() & 3) << 1)) & 3;
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x4000) != 0 && D_801EB630[arg0].unk_aa < 2) {
    *mode = 0;
  }
  switch (*mode) {
  case 0:
    *mode = 1;
    break;
  case 1:
    if ((D_801EB630[arg0].unk_80 & 0x80) != 0) {
      *mode = 2;
      D_801EB630[arg0].unk_100 |= 2;
    } else {
      *mode = 1;
    }
    break;
  case 2:
    if ((D_801EB630[arg0].unk_80 & 0x40) != 0) {
      *mode = 3;
    } else {
      *mode = 1;
    }
    break;
  case 3:
    if ((D_801EB630[arg0].unk_80 & 0x20) != 0) {
      *mode = 4;
      D_801EB630[arg0].unk_f6 = D_801EB630[arg0].unk_8c[rand() & 7];
    } else {
      *mode = 1;
    }
    break;
  default:
    break;
  }
  if (((D_801EB630[arg0].unk_82 & 0x20) != 0) ||
      ((D_801EB630[arg0].unk_104 & 0x4000) != 0)) {
    BATTLE_GLOBAL_BYTE_6375 = 1;
    if (func_800A9304((u8)(arg0 + 3)) != 0) {
      pickTargetStoreGlobal(arg0);
    }
  } else if (func_800A9304((u8)(arg0 + 3)) != 0) {
    D_80146384 = func_801E2A88(arg0);
  }
  if (BATTLE_GLOBAL_BYTE_62EA != 0) {
    work = &D_801EB630[arg0];
    battleCurrentEnemyWorkState = work;
    BATTLE_ENEMY_SCRATCH_PTR = work;
    work->unk_e4(0);
  }
  if (D_80146384 == 0xFF) {
    BATTLE_GLOBAL_BYTE_6375 = 0;
    D_801EB630[arg0].unk_f5 = 0;
  } else {
    D_801EB630[arg0].unk_f5 = BATTLE_GLOBAL_BYTE_6375;
  }
}
