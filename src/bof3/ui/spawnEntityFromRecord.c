#include "bof3/ui/game00_internal.h"

/* Per-entity spawn fields inside the 0x98-byte spawn-record array and the
 * spawn-gate byte table. Each address is its own canonical label, so the
 * compiler emits the original per-access `lui %hi` / `addu $at,$at,index` /
 * `store %lo($at)` triple instead of one shared base. */
extern u8 D_80146900[];
extern u8 D_80146904[];
extern u8 D_80146908[];
extern u8 D_8014690C[];
extern u8 D_80146910[];
extern u8 D_8014691C[];
extern u8 D_80144FC4[];

extern void func_801A4AA8(void);
extern void func_8014DD3C(u16 entry);
extern s16  func_8015477C(s32 x, s32 y);
extern void func_801A4990(const volatile u8* data);
extern void func_801A4BC0(s16 x, s16 y, u32 size);
extern void func_8014D6B8(u32 flag);

#define ENTITY_SLOT_COUNT 30
#define ENTITY_SLOT_SIZE  0x98

/* Scratchpad spawn counter and packed entry word, read through the reviewed
 * D_1F800000 symbol so the original lui/addiu pair is reproduced. The counter
 * is tested and indexed through its signed halfword view; only the final
 * increment reads it unsigned. */
#define ENTITY_COUNT_S16 (counter_state->count)
#define ENTITY_COUNT_U16 (*(u16*)&(((struct SpawnScratchState*)D_1F800000)->count))
#define ENTITY_ENTRY_DATA (((struct SpawnScratchState*)D_1F800000)->entry_data)

/* Persistent per-entity spawn field of the 0x98-byte record array at
 * 0x80146888. Each field is addressed through its own canonical symbol so the
 * compiler emits the original per-store address triple and re-reads the
 * counter for every one of these writes. */
#define ENTITY_SLOT_FIELD(type, symbol)                                        \
  PSX_REF(type, (u8*)symbol + (u32)ENTITY_COUNT_S16 * ENTITY_SLOT_SIZE)

/* Spawn-gate byte table addressed through its canonical label; same access as
 * GAME_SPAWN_GATE_BYTE, spelled through the symbol so the address triple
 * matches the original. */
#define SPAWN_GATE_BYTE(index)                                                 \
  PSX_REF(volatile u8, (u8*)D_80144FC4 + (u32)(index))

/* @behavior initializes the next free entity slot from one packed spawn
 * record, applies its saved-state gate, and advances the entity count.
 * @source 0x801A7330
 * @status partial
 * @match 99.59
 * @residual Native comparison: 244/245 instructions at the original 980 bytes.
 * The late packed-word OR swaps its two source operands at +0x2C0. The spawn
 * record remains volatile to preserve repeated byte samples, including spawn[0],
 * spawn[3] and spawn[0xC].
 */
void spawnEntityFromRecord(const volatile u8* spawn) {
  struct SpawnScratchState {
    s16 count;
    u16 entry_data;
  };
  struct SpawnScratchState* counter_state;
  struct GameWorkArea* work;
  struct GameWorkArea* field_work;
  s16                  slot;
  s16                  field_slot;
  s16                  state;
  s16                  dist;
  s32                  coord_x;
  s32                  coord_y;
  s32                  result;
  u32                  packed_entry;
  u32                  x_offset;
  u32                  y_offset;
  u8                   x_fraction;
  u8                   y_fraction;
  u8                   check;
  u8                   route_byte;

  counter_state = (struct SpawnScratchState*)D_1F800000;
  slot = ENTITY_COUNT_S16;
  if (slot >= ENTITY_SLOT_COUNT) {
    return;
  }

  {
    struct GameWorkArea* selected_work;

    selected_work = (struct GameWorkArea*)((u8*)D_80146888 +
                                         (u32)slot * ENTITY_SLOT_SIZE);
    g_game_work = selected_work;
    GAME_WORK_AREA_PTR = selected_work;
  }
  ENTITY_ENTRY_DATA = (u16)(((u32)spawn[1] << 8) | spawn[2]);

  func_801A4AA8();
  func_8014DD3C(ENTITY_ENTRY_DATA);

  g_game_work->flags_00 = 1;

  do {
    coord_x = (s32)spawn[5] << 16;
    x_offset = (u32)ENTITY_COUNT_S16 * ENTITY_SLOT_SIZE;
  } while (0);
  x_fraction = spawn[4];
  work = g_game_work;
  if (x_fraction != 0) {
    coord_x |= 0x8000;
  }
  work->coord_x_34 = coord_x;
  PSX_REF(s32, (u8*)D_80146908 + x_offset) = coord_x;

  do {
    coord_y = (s32)spawn[7] << 16;
    y_offset = (u32)ENTITY_COUNT_S16 * ENTITY_SLOT_SIZE;
    y_fraction = spawn[6];
  } while (0);
  if (y_fraction != 0) {
    coord_y |= 0x8000;
  }
  work->coord_y_38 = coord_y;
  PSX_REF(s32, (u8*)D_8014690C + y_offset) = coord_y;

  ENTITY_SLOT_FIELD(s32, D_80146910) = 0;

  state = 6;
  dist = func_8015477C(work->coord_x_34, work->coord_y_38);

  {
    struct GameWorkArea* initialized_work;
    struct GameWorkArea* flags_work;

    initialized_work = g_game_work;
    initialized_work->unk_01 = state;
    flags_work = g_game_work;
    initialized_work->counter_3E = dist;
    do {
      flags_work->flags_02 = 0;
    } while (0);
  }
  route_byte = spawn[0];
  g_game_work->route_index_08 = route_byte & 0xF;
  g_game_work->unk_06 = spawn[0] >> 4;

  ENTITY_SLOT_FIELD(u8, D_80146900) = 0;
  ENTITY_SLOT_FIELD(u8, D_8014691C) = 0x7F;

  func_801A4990(spawn + 0xB);

  if ((spawn[3] & 0x80) != 0) {
    g_game_work->flags_00 |= 0x20;
  }

  g_game_work->unk_5D = 0;
  g_game_work->unk_5F = 0;
  g_game_work->unk_5E = 0;
  g_game_work->flags_5C = spawn[3] & 0xF;
  g_game_work->unk_29 = state;

  field_slot = ENTITY_COUNT_S16;
  field_work = g_game_work;
  PSX_REF(u16, (u8*)D_80146904 +
                   (u32)field_slot * ENTITY_SLOT_SIZE) = 0;
  do {
    packed_entry = spawn[9];
    packed_entry <<= 8;
    packed_entry |= spawn[0xA];
    field_work->unk_18 = (s32)packed_entry;
  } while (0);
  field_work->field_05 = spawn[8];
  g_game_work->field_0B = spawn[0xC];
  g_game_work->unk_2A = (u8)((g_game_work->unk_07 >> 4) & 1);

  work = g_game_work;
  check = work->field_05;
  if (((SPAWN_GATE_BYTE(check >> 3) >> (check & 7)) & 1) != 0) {
    if ((spawn[0xC] & 1) != 0) {
      g_game_work->flags_00 = 0;
      return;
    }
    if ((u16)g_game_work->coord_x_34 == 0 &&
        (u16)g_game_work->coord_y_38 == 0) {
      func_801A4BC0(FIELD_REF(s16, g_game_work, 0x36u),
                    FIELD_REF(s16, g_game_work, 0x3Au), 0x10);
    }
    result = 1;
  } else {
    result = 0;
  }

  func_8014D6B8(result);

  ENTITY_COUNT_U16 = ENTITY_COUNT_U16 + 1;
}
