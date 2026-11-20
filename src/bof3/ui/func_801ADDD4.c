#include "bof3/ui/game00_internal.h"

/* One 8-byte row of the 0x801CB8DC level table: the u16 exp threshold followed
 * by the six growth bytes (two whole bytes, four packed nibble pairs, then the
 * two trailing id bytes). Rows are 99 per character, so the character stride is
 * 99 * 8 = 0x318. This target's review binds the same table as data.LevelObject
 * (0x15A8 = 693 rows of 8, 693 = 7 * 99); the shared field names live in
 * include/data/equipment.h, where the trailing pair is `ability` then `unk_07`,
 * so only the packed nibbles are unnamed here. */
typedef struct GameLevelRow {
  u16 exp;
  u8  growth[6];
} GameLevelRow;

extern GameLevelRow D_801CB8DC[][99];

/* Character record fields of the mutable stat block, addressed through the
 * reviewed 0x80144968 record symbol (stride 0xA4 bytes, selected by the low
 * byte of the character id). */
#define CHARACTER_FIELD(type, byte_offset)                                     \
  PSX_REF(type, (u8*)D_80144968 + (u32)(byte_offset))
#define CHARACTER_EXP(x)    CHARACTER_FIELD(volatile s32, 8 + (x))
#define CHARACTER_LEVEL(x)  CHARACTER_FIELD(volatile u8, 6 + (x))
#define CHARACTER_STATS(x)  CHARACTER_FIELD(volatile u16, 0x3C + (x))
#define CHARACTER_RATES(x)  CHARACTER_FIELD(s8, 0x85 + (x))
#define CHARACTER_RECORD(x) PSX_PTR(volatile u8, (u8*)D_80144968 + (u32)(x))

#define CHARACTER_STRIDE 0xA4
#define LEVEL_MAX        99
#define STAT_MAX         1000
#define STAT_CLAMP       999

/* @behavior applies each newly reached level's packed growth row to the mutable
 * character record, clamps the six base stats to 0..999, registers both ability
 * bytes, then recalculates the character.
 * @source 0x801ADDD4
 * @see docs/specs/data/characters.md
 * @status partial
 * @match 79.04
 * @residual 215/272 instructions; 1088 original bytes versus 1076 current,
 * with a 48-byte frame on both. The growth-loop record stride still shares
 * the early record offset instead of being recomputed. Entry and saved
 * registers, preparation order, the last clamp's argument setup and tail
 * address forms remain different.
 */
void func_801ADDD4(s32 arg0) {
  u8  character;
  s32 character_offset;
  s32 exp;
  s32 accumulator;
  u8  new_level;
  u8  current_level;
  u8  level;
  s32 value;

  character = (u8)arg0;
  new_level = 1;
  accumulator = 0;

  character_offset = character * CHARACTER_STRIDE;
  exp = CHARACTER_EXP(character_offset);

  while (new_level < LEVEL_MAX) {
    accumulator += D_801CB8DC[character][new_level].exp;
    if (exp < accumulator) {
      break;
    }
    new_level++;
  }

  current_level = CHARACTER_LEVEL(character_offset);
  if (current_level == LEVEL_MAX) {
    return;
  }
  if ((s32)new_level - (s32)current_level <= 0) {
    return;
  }

  /* Each stat clamp stores 999 through the `value` local instead of the
   * macro constant so the constant is materialised per site, which is the
   * `li v0,999` the original emits in each clamp's branch delay slot. */
  if ((u32)current_level < (u32)new_level) {
    level = current_level;
    do {
      accumulator = (s32)D_801CB8DC[character][level].growth[0] +
               (s32)CHARACTER_RATES(character_offset);
      if (accumulator < 0) {
        accumulator = 0;
      }
      value = (s32)CHARACTER_STATS(character_offset) + accumulator;
      CHARACTER_STATS(character_offset) = (u16)value;
      if ((u16)value >= STAT_MAX) {
        value = STAT_CLAMP;
        CHARACTER_STATS(character_offset) = (u16)value;
      }

      accumulator = (s32)D_801CB8DC[character][level].growth[1] +
               (s32)CHARACTER_RATES(1 + character_offset);
      if (accumulator < 0) {
        accumulator = 0;
      }
      value = (s32)CHARACTER_STATS(2 + character_offset) + accumulator;
      CHARACTER_STATS(2 + character_offset) = (u16)value;
      if ((u16)value >= STAT_MAX) {
        value = STAT_CLAMP;
        CHARACTER_STATS(2 + character_offset) = (u16)value;
      }

      accumulator = (s32)(D_801CB8DC[character][level].growth[2] >> 4) +
               (s32)CHARACTER_RATES(2 + character_offset);
      if (accumulator < 0) {
        accumulator = 0;
      }
      value = (s32)CHARACTER_STATS(4 + character_offset) + accumulator;
      CHARACTER_STATS(4 + character_offset) = (u16)value;
      if ((u16)value >= STAT_MAX) {
        value = STAT_CLAMP;
        CHARACTER_STATS(4 + character_offset) = (u16)value;
      }

      accumulator = (s32)(D_801CB8DC[character][level].growth[2] & 0x0F) +
               (s32)CHARACTER_RATES(3 + character_offset);
      if (accumulator < 0) {
        accumulator = 0;
      }
      value = (s32)CHARACTER_STATS(6 + character_offset) + accumulator;
      CHARACTER_STATS(6 + character_offset) = (u16)value;
      if ((u16)value >= STAT_MAX) {
        value = STAT_CLAMP;
        CHARACTER_STATS(6 + character_offset) = (u16)value;
      }

      accumulator = (s32)(D_801CB8DC[character][level].growth[3] >> 4) +
               (s32)CHARACTER_RATES(4 + character_offset);
      if (accumulator < 0) {
        accumulator = 0;
      }
      value = (s32)CHARACTER_STATS(8 + character_offset) + accumulator;
      CHARACTER_STATS(8 + character_offset) = (u16)value;
      if ((u16)value >= STAT_MAX) {
        value = STAT_CLAMP;
        CHARACTER_STATS(8 + character_offset) = (u16)value;
      }

      accumulator = (s32)(D_801CB8DC[character][level].growth[3] & 0x0F) +
               (s32)CHARACTER_RATES(5 + character_offset);
      if (accumulator < 0) {
        accumulator = 0;
      }
      value = (s32)CHARACTER_STATS(0x0A + character_offset) + accumulator;
      CHARACTER_STATS(0x0A + character_offset) = (u16)value;
      if ((u16)value >= STAT_MAX) {
        value = STAT_CLAMP;
        CHARACTER_STATS(0x0A + character_offset) = (u16)value;
      }

      func_801651DC(D_801CB8DC[character][level].growth[4], character, 0, 0);
      func_801651DC(D_801CB8DC[character][level].growth[5], character, 0, 0);
      level++;
    } while (level < new_level);
  }

  CHARACTER_LEVEL((s32)character * CHARACTER_STRIDE) = new_level;
  func_80164A44(CHARACTER_RECORD((s32)character * CHARACTER_STRIDE));
}
