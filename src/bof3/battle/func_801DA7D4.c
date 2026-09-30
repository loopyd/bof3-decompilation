#include "bof3/battle/battle03_internal.h"

typedef struct Battle03GroupQueueEntry {
  s16 value;
  u16 index;
} Battle03GroupQueueEntry;

/* @source 0x801DA7D4
 * @behavior Rebuilds the pending group queue from the eligible battler slots:
 * it sums the enemy slot values at `D_801EB6D8[index - 3]` and keeps the largest
 * one for the slots 3..10 that pass the 0x801EB734 flag-word bit 0x8000, then
 * collects the eligible local slots 0..2 (values at `D_80145F28`) into a stack
 * list ordered by descending value, clears the three bytes of `groupQueueTable`
 * to the 0xFF empty marker, resets `groupQueueCursor` and appends the
 * still-eligible slot numbers to the queue.
 * @status partial
 * @match 55.61
 * @residual 109/196 instructions, 784 original bytes versus 776 current. The
 * first live difference is the phase-1 accumulator: the original retains an
 * unconditional `total += value` (sunk into the `beqz` delay slot) whose value is
 * never read, while the canonical gcc-2.7.2-psx dead-insn pass removes the dead
 * accumulation and cascades into the s0/s1/s2/s3 allocation. Every other shape
 * (queue fill, descending sort, cursor drain) is present. Tried and rejected with
 * the same 109/196 or unchanged allocation: conditional vs unconditional
 * accumulate, shared u8 sum/counter, u16 sum with a separate u8 counter, and the
 * `total = maximum` spelling; gcc-2.6.3-psx also removes the accumulation.
 * Smallest missing evidence: the retail translation unit's dead-code-surviving
 * shape for that accumulator (or a per-object profile/variant rung, which this
 * mission does not authorize).
 */
void func_801DA7D4(void) {
  Battle03GroupQueueEntry entries[4];
  u16 total;
  u16 maximum;
  u8  count;
  u8  slots;
  u8  index;
  s8* cursor;

  total = 0u;
  maximum = 0u;
  index = 3u;
  while (index < 0x0bu) {
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((D_801EB734[index - 3u].word_00 & 0x8000u) != 0u)) {
      u16 value;

      if (func_801DB9E4(index) != 0u) {
        value = D_801EB6D8[index - 3u].half_00;
        total += value;
        if (maximum < value) {
          maximum = value;
        }
      }
    }
    index += 1u;
  }
  count = 0u;
  index = 0u;
  while (index < 3u) {
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((D_80145FB8[index].flags_00 & 0x8000u) != 0u)) {
      u16 value;

      if (func_801DB844(index) != 0u) {
        value = D_80145F28[index].half_00;
        entries[count].index = index;
        entries[count].value = value;
        count += 1u;
      }
    }
    index += 1u;
  }

  slots = count;
  if (count > 1u) {
    u8 left;

    left = 0u;
    while (left + 1u < slots) {
      u8 right;

      right = left + 1u;
      while (right < slots) {
        if (entries[left].value < entries[right].value) {
          Battle03GroupQueueEntry temp;

          temp = entries[left];
          entries[left] = entries[right];
          entries[right] = temp;
        }
        right += 1u;
      }
      left += 1u;
    }
  }

  index = 0u;
  while (index < 3u) {
    groupQueueTable[index] = 0xffu;
    index += 1u;
  }

  cursor = (s8*)&groupQueueCursor;
  *cursor = 0;

  index = 0u;
  while (index < slots) {
    if (func_801DB844((u8)entries[index].index) != 0u) {
      groupQueueTable[*cursor] = entries[index].index;
      *(u8*)cursor = *(u8*)cursor + 1;
    }
    index += 1u;
  }
}
