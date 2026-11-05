#include "bof3/battle/battle03_internal.h"

/* @source 0x801E16B4
 * @behavior Entry 0 of the cast-voice class table at 0x801EB224. Once the
 * EXE-side EMI loader reports ready it arms the cast-voice countdown byte +0x09
 * of the scratch battle work from the trigger scan of the current actor: while
 * the published local work record byte +0x119 reads 4, a record without flag
 * 0x2 stores 0x1E when the scan submits and 0 when it does not, while a record
 * with flag 0x2 scans only when the actor byte at 0x801463C9 equals that same
 * byte or reads 7 or 8 and stores 0 when the scan submits; every other byte
 * +0x119 value stores 0. It then selects the current kind from the global
 * halfword 0x801463C0: a kind without the 0x800 mask re-arms the countdown from
 * the byte table at 0x801EB23C indexed by the actor byte 0x801463C9 or the
 * table at 0x801EB230 indexed by record byte +0x79, as the record flag 0x2
 * selects, submits the effect id scratch byte +0x08 + 0x2C through
 * func_8014D8D4 and advances scratch battle work byte +0x02; a kind with that
 * mask instead advances scratch battle work byte +0x01. Both paths finish with
 * the second local readiness helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initCastVoiceCountdownWhenTriggered(void) {
  Battle03LocalWork* work;
  Battle03LocalWork* tail;

  if (func_80162D00() != 0) {
    work = D_80146250;
    if (work->unk_119 == 4u) {
      if ((work->unk_128 & 0x2u) == 0u) {
        if (scanTriggerTableSubmit(work->unk_79, D_801463CD) != 0u) {
          D_1F800044->unk_09 = 0;
        } else {
          D_1F800044->unk_09 = 0x1E;
        }
      } else if (BATTLE_GLOBAL_BYTE_63C9 == work->unk_119 ||
                 (BATTLE_GLOBAL_BYTE_63C9 >= 7u &&
                  BATTLE_GLOBAL_BYTE_63C9 <= 8u)) {
        if (scanTriggerTableSubmit(work->unk_79, D_801463CD) != 0u) {
          D_1F800044->unk_09 = 0;
        }
      }
    } else {
      D_1F800044->unk_09 = 0;
    }
    if ((D_801CA71C[BATTLE_GLOBAL_HALF_63C0].mask_00 & 0x800u) == 0u) {
      tail = D_80146250;
      if ((tail->unk_128 & 0x2u) != 0u) {
        D_1F800044->unk_09 = D_801EB23C[BATTLE_GLOBAL_BYTE_63C9];
      } else {
        D_1F800044->unk_09 = D_801EB230[tail->unk_79];
      }
      func_8014D8D4(D_1F800044->unk_08 + 0x2Cu);
      D_1F800044->unk_02++;
    } else {
      D_1F800044->unk_01++;
    }
    localReadyOrHelper2();
  }
}
