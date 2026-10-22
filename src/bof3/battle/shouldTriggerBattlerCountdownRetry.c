#include "bof3/battle/battle03_internal.h"

/* @source 0x801DDCB4
 * @behavior Decides whether the countdown of the battler selected by arg0 fires
 * on this frame: the countdown byte is local work +0x121 for the first three
 * battlers and enemy work +0xFD otherwise, a zero countdown never fires, a roll
 * of rand() % 100 at or below the rate byte keeps the countdown (the
 * battleCommandEntryTable byte +8 selected by the countdown byte for the
 * countdown values 1 and 2, the fixed 0x4C bound otherwise), and the retry roll
 * compares rand() % 10000 with 10000 minus 50 times the entry byte of
 * battleCommandEntryTable selected by the retry index byte (local work +0xA6
 * or enemy work +0xB6).
 * @status partial
 * @match 87.72
 * @residual Live asm diff 100/114 instructions, 456 versus 456 bytes. The
 * countdown read, the zero test, both random rolls, the 100/10000 modulo
 * expansions, both table reads and the hoisted battleCommandEntryTable base
 * (`lui`/`addiu` pair plus one `addu` per index arm) reproduce. The residual is
 * allocation and block layout: the enemy record address is hoisted before the
 * retry split with -3 * 0x118 folded into the base constant (`addiu` -19736 =
 * 0x801EB2E8) while +0xB6 stays the load displacement (`lbu v0,182(v1)`); the
 * two retry-table sums are `addu v0,a1,v0` where the original emits
 * `addu v0,v0,a1`; and the countdown-table (`counter < 3`) arm inverts the
 * shared-return branch (`bnez v0,0x801ddd9c` to the continuation where the
 * original branches to the epilogue with `beqz v0,0x801dde64`); the `beqz`
 * target of the index split shifts with the hoist. Building the record address
 * inside the retry arm removes the hoist but rotates the second mask to v1 and
 * both `mfhi` results to a1: every measured in-arm form reaches 93/114, so the
 * hoist is what earns the 100. Making the retry edge an explicit jump leaves the
 * branch direction unchanged (100/114) and a merged rate predicate costs 7
 * (93/114, 452 bytes), so the direction has no clean-C handle. The hoisted
 * record base is spelled through the flat map symbol at 0x801EB2E8 for the
 * unreduced battler, the same address as `&D_801EB630[(arg0 & 0xff) - 3u]` but
 * inside the 0x118-byte record array. Rungs 4 and 5 are unauthorized here.
 */

extern int rand(void);

u8 shouldTriggerBattlerCountdownRetry(u32 arg0) {
  u8  counter;
  s32 roll;
  s32 chance;
  s32 index;
  u8 *table;
  Battle03EnemyWork *enemy;

  if ((arg0 & 0xff) < 3u) {
    counter = D_80145E90[arg0 & 0xff].unk_121;
  } else {
    counter = D_801EB630[(arg0 & 0xff) - 3u].unk_fd;
  }
  if (counter == 0) {
    return 0u;
  }

  roll = rand() % 100;
  if (counter < 3u) {
    if (roll <= battleCommandEntryTable[counter + 8]) {
      return 1u;
    }
  } else if (roll < 0x4c) {
    return 1u;
  }

  table = battleCommandEntryTable;
  /* D_801EB2E8 holds 0x118-byte records, so battler * 0x118 == this shift
   * chain and battler 3 is D_801EB630. */
  enemy = (Battle03EnemyWork*)(D_801EB2E8 +
                              (((arg0 & 0xff) * 36u - (arg0 & 0xff)) * 8u));
  if ((arg0 & 0xff) < 3u) {
    index = FIELD_REF(u8, &D_80145E90[arg0 & 0xff], 0xa6u);
  } else {
    index = FIELD_REF(u8, enemy, 0xb6u);
  }

  chance = 10000 - (s32)table[index] * 50;
  return chance >= (s32)(rand() % 10000);
}
