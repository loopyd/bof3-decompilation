#include "bof3/bof3.h"

void func_801E3160(void);

/* Kind record table at 0x801CA71C: 0x14-byte records whose leading halfword is
 * the kind mask the battle engine tests (bit 0x800 included). */
typedef struct Boss05516KindRecord {
  u16 mask;
  u8  pad_02[0x12];
} Boss05516KindRecord;

extern u16 D_801463C0;                   /* @source 0x801463C0 @kind unknown */
extern Boss05516KindRecord D_801CA71C[]; /* @source 0x801CA71C @kind unknown */

/* @source 0x800C21E4
 * @behavior Overlay entry point (its word sits in the overlay's own pointer
 * table at 0x800C35E0): reads the current battle id halfword 0x801463C0,
 * scales it by the 0x14-byte kind-record stride into the kind table at
 * 0x801CA71C, tests bit 0x800 of that record's leading mask halfword, and
 * calls the EXE-side enemy-readiness helper at 0x801E3160 (enemyReadyOrHelper2
 * of emi/battle/battle/03) only when the bit is clear; that helper's result is
 * discarded. The identical 0x800 test on the identical table is performed by
 * func_801E1A14 of emi/battle/battle/03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEnemyReadyUnlessKindFlagBossBoss05516_800C21E4(void) {
  if ((D_801CA71C[D_801463C0].mask & 0x800) == 0) {
    func_801E3160();
  }
}
